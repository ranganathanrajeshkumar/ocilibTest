// -----------------------------------------------------------------------------
#include "TMyOracle.h"	
#include "ocilib.hpp"
#include "TMyOracleResultSet.h"
#include "utils.h"
// -----------------------------------------------------------------------------

TMyOracle::TMyOracle(OCI_TYPE type)
	: m_Connection{ nullptr }, m_type{ type }, m_lst_query{}, m_lst_error{}
{
	if (m_type == OCI_TYPE::OCI_CXX_API)
	{
		m_mutex = ocilib::Mutex::Create();
	}
}

// -----------------------------------------------------------------------------
TMyOracle::~TMyOracle()
{
	Disconnect();	
	if (m_type == OCI_TYPE::OCI_CXX_API)
	{
		ocilib::Mutex::Destroy(m_mutex);
	}
}
// -----------------------------------------------------------------------------

bool TMyOracle::Connect(const std::string& user, const std::string& password, const std::string& db)
{
	// Disconnect if already connected
	Disconnect();

	try
	{		
		if (m_type == OCI_TYPE::OCI_CXX_API)
		{
			// Create a new OCI C++ environment
			m_conn = std::make_unique<Connection>(db, user, password, Environment::SessionDefault);
			if (!m_conn || m_conn->IsNull())
			{
				std::cerr << "[FATAL] TMyOracle::Connect: Failed to connect to database: " << std::endl;
				return false;
			}
			
			m_conn->SetAutoCommit(true);
			m_conn->SetStatementCacheSize(10);

			std::cout << "Connected to database: " << db << std::endl;
			return true;
		}
		else if (m_type == OCI_TYPE::OCI_C_API)
		{
			// Create a new OCI environment
			m_Connection = OCI_ConnectionCreate(db.c_str(), user.c_str(), password.c_str(), OCI_SESSION_DEFAULT);
			if (!m_Connection)
			{
				std::cerr << "[FATAL] TMyOracle::Connect: Failed to connect to database: " << OCI_ErrorGetString(OCI_GetLastError()) << std::endl;
				return false;
			}

			// Set auto-commit mode
			OCI_SetAutoCommit(m_Connection, true);

			// Set the statement cache size
			OCI_SetStatementCacheSize(m_Connection, 10);

			std::cout << "Connected to database: " << db << std::endl;

			return true;
		}

	}	
	catch (const std::exception& ex)
	{
		std::cerr << "[EXCEPTION] TMyOracle::Connect: " << ex.what() << std::endl;
	}

	return false;
	
}
// -----------------------------------------------------------------------------
void TMyOracle::Disconnect()
{
	if (m_type == OCI_TYPE::OCI_CXX_API)
	{
		if (m_conn)
		{
			if (!m_conn->IsNull())
			{
				// Disconnect from the database
				m_conn->Close();
				m_conn.reset();
				std::cout << "Disconnected from database" << std::endl;
			}
		}
	}
	else if (m_type == OCI_TYPE::OCI_C_API)
	{
		if (m_Connection)
		{
			if (OCI_IsConnected(m_Connection))
			{
				// Disconnect from the database
				OCI_ConnectionFree(m_Connection);
				m_Connection = nullptr;
				std::cout << "Disconnected from database" << std::endl;
			}
		}
	}
}
// -----------------------------------------------------------------------------
bool TMyOracle::IsConnected() const
{
	//ocilib::Mutex::Acquire(m_mutex);

	bool result = false;

	if (m_type == OCI_TYPE::OCI_C_API)
	{
		if (m_Connection)
		{
			result = OCI_IsConnected(m_Connection);
		}
	}
	else if (m_type == OCI_TYPE::OCI_CXX_API)
	{
		if (m_conn)
		{
			result = (!m_conn->IsNull() && m_conn->IsServerAlive());
		}
	}
	
	//ocilib::Mutex::Release(m_mutex);
	return result;
}
// -----------------------------------------------------------------------------
TMyOracleResultSet* TMyOracle::ExecuteQuery(const std::string& query)
{	
	if (query.empty())
	{
		std::cerr << "Query is empty" << std::endl;
		return nullptr;
	}

	//ocilib::Mutex::Acquire(m_mutex);
	try
	{
		if (m_type == OCI_TYPE::OCI_C_API)
		{
			if (!m_Connection)
			{
				std::cerr << "Not connected to database" << std::endl;
				//ocilib::Mutex::Release(m_mutex);
				return nullptr;
			}
			// Create a new statement
			TMyOracleStatement stmt(m_Connection);
			if (!stmt)
			{
				std::cerr << "Failed to create statement" << std::endl;
				//ocilib::Mutex::Release(m_mutex);
				return nullptr;
			}
			// Prepare and execute the statement
			if (!OCI_Prepare(stmt, query.c_str()))
			{
				m_lst_error = OCI_ErrorGetString(OCI_GetLastError());
				std::cerr << "Failed to prepare statement: " << m_lst_error << std::endl;
				//ocilib::Mutex::Release(m_mutex);
				return nullptr;
			}
			// Execute the statement
			if (!OCI_Execute(stmt))
			{
				m_lst_error = OCI_ErrorGetString(OCI_GetLastError());
				std::cerr << "Failed to execute statement: " << m_lst_error << std::endl;
				OCI_StatementFree(stmt);
				//ocilib::Mutex::Release(m_mutex);
				return nullptr;
			}

			m_lst_query = OCI_GetSql(stmt);
			
			// Commit the transaction
			OCI_Commit(m_Connection);

			// Get the result set
			TMyOracleResultSet* result_set = TMyOracleResultSet::ExtractResultSet(OCI_GetResultset(stmt));

			// Check if the result set is valid and return it
			if (result_set)
			{
				//ocilib::Mutex::Release(m_mutex);
				return result_set;
			}
		}
		else if (m_type == OCI_TYPE::OCI_CXX_API)
		{
			if (!m_conn || m_conn->IsNull() || !m_conn->IsServerAlive())
			{
				std::cerr << "Not connected to database" << std::endl;
				//ocilib::Mutex::Release(m_mutex);
				return nullptr;
			}

			// Create a new statement
			Statement stmt(*m_conn);
			if (stmt.IsNull())
			{
				std::cerr << "Failed to create statement" << std::endl;
				//ocilib::Mutex::Release(m_mutex);
				return nullptr;
			}

			stmt.Prepare(query);
			
			// Execute the statement
			stmt.ExecutePrepared();
			
			m_lst_query = stmt.GetSql();
						
			// Commit the transaction
			m_conn->Commit();
			
			ocilib::Resultset rs = stmt.GetResultset();
			if (rs.IsNull())
			{
				std::cerr << "Failed to get result set" << std::endl;
				//ocilib::Mutex::Release(m_mutex);
				return nullptr;
			}

			// Get the result set
			TMyOracleResultSet* result_set = TMyOracleResultSet::ExtractResultSet(&rs);
			
			// Check if the result set is valid and return it
			if (result_set)
			{
				//ocilib::Mutex::Release(m_mutex);
				return result_set;
			}
		}
	}
	catch (const std::exception& ex)
	{
		m_lst_error = OCI_ErrorGetString(OCI_GetLastError());

		std::cerr << "[EXCEPTION] TMyOracle::ExecuteQuery: " << ex.what() << std::endl;
	}

	// Rollback in case of error
	OCI_Rollback(m_Connection);
	
	//ocilib::Mutex::Release(m_mutex);

	return nullptr;
}