// -----------------------------------------------------------------------------
#include "TMyOracle.h"	
#include "ocilib.hpp"
#include "TMyOracleResultSet.h"
#include "utils.h"
#include <atomic>
// -----------------------------------------------------------------------------
static std::atomic<int> g_conn_instance_counter{ 0 };
// -----------------------------------------------------------------------------

TMyOracle::TMyOracle(OCI_TYPE type)
	: m_Connection{ nullptr }, m_type{ type }, m_lst_query{}, m_lst_error{}
{
	m_mutex = OCI_MutexCreate();
}

// -----------------------------------------------------------------------------
TMyOracle::~TMyOracle()
{
	Disconnect();	
	
	OCI_MutexFree(m_mutex);	
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
				std::cerr << "[FATAL] TMyOracle::Connect: Failed to connect [" << m_conn_instance_counter << "] to database: " << std::endl;
				return false;
			}
			
			m_conn->SetAutoCommit(true);
			m_conn->SetStatementCacheSize(10);

			m_conn_instance_counter = ++g_conn_instance_counter;
			std::cout << "[" << m_conn_instance_counter << "] Connected to database: " << db << std::endl;
			return true;
		}
		else if (m_type == OCI_TYPE::OCI_C_API)
		{
			// Create a new OCI environment
			m_Connection = OCI_ConnectionCreate(db.c_str(), user.c_str(), password.c_str(), OCI_SESSION_DEFAULT);
			if (!m_Connection)
			{
				std::cerr << "[FATAL] TMyOracle::Connect: Failed to connect [" << m_conn_instance_counter << "] to database: " << OCI_ErrorGetString(OCI_GetLastError()) << std::endl;
				return false;
			}

			// Set auto-commit mode
			OCI_SetAutoCommit(m_Connection, true);

			// Set the statement cache size
			OCI_SetStatementCacheSize(m_Connection, 10);

			std::cout << "[" << m_conn_instance_counter << "] Connected to database: " << db << std::endl;

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
				std::cout << "[" << m_conn_instance_counter << "] Disconnected from database" << std::endl;
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
				std::cout << "[" << m_conn_instance_counter << "] Disconnected from database" << std::endl;
			}
		}
	}
}
// -----------------------------------------------------------------------------
bool TMyOracle::IsConnected() const
{
	OCI_MutexAcquire(m_mutex);

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
	
	OCI_MutexRelease(m_mutex);
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
	
	auto FetchRecords = [this](const std::string& query) -> TMyOracleResultSet*
	{
		try
		{
			if (m_type == OCI_TYPE::OCI_C_API)
			{
				if (!m_Connection)
				{
					std::cerr << "[" << m_conn_instance_counter << "] Not connected to database" << std::endl;

					return nullptr;
				}
				// Create a new statement
				TMyOracleStatement stmt(m_Connection);
				if (!stmt)
				{
					std::cerr << "[" << m_conn_instance_counter << "] Failed to create statement" << std::endl;

					return nullptr;
				}
				// Prepare and execute the statement
				if (!OCI_Prepare(stmt, query.c_str()))
				{
					m_lst_error = OCI_ErrorGetString(OCI_GetLastError());
					std::cerr << "[" << m_conn_instance_counter << "] Failed to prepare statement: " << m_lst_error << std::endl;

					return nullptr;
				}
				// Execute the statement
				if (!OCI_Execute(stmt))
				{
					m_lst_error = OCI_ErrorGetString(OCI_GetLastError());
					std::cerr << "[" << m_conn_instance_counter << "] Failed to execute statement: " << m_lst_error << std::endl;
					OCI_StatementFree(stmt);

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

					return result_set;
				}
			}
			else if (m_type == OCI_TYPE::OCI_CXX_API)
			{
				if (!m_conn || m_conn->IsNull() || !m_conn->IsServerAlive())
				{
					std::cerr << "[" << m_conn_instance_counter << "] Not connected to database" << std::endl;

					return nullptr;
				}

				// Create a new statement
				Statement stmt(*m_conn);
				if (stmt.IsNull())
				{
					std::cerr << "[" << m_conn_instance_counter << "] Failed to create statement" << std::endl;

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
					std::cerr << "[" << m_conn_instance_counter << "] Failed to get result set" << std::endl;

					return nullptr;
				}

				// Get the result set
				TMyOracleResultSet* result_set = TMyOracleResultSet::ExtractResultSet(&rs);

				// Check if the result set is valid and return it
				if (result_set)
				{
					return result_set;
				}
			}
		}
		catch (const std::exception& ex)
		{
			m_lst_error = OCI_ErrorGetString(OCI_GetLastError());

			std::cerr << "[EXCEPTION] TMyOracle::ExecuteQuery[" << m_conn_instance_counter << "]: " << ex.what() << std::endl;
		}
		catch (...)
		{
			m_lst_error = OCI_ErrorGetString(OCI_GetLastError());
			std::cerr << "[EXCEPTION] TMyOracle::ExecuteQuery[" << m_conn_instance_counter << "]: Unknown error: " << m_lst_error << std::endl;
		}

		if (m_type == OCI_TYPE::OCI_C_API)
		{
			// Rollback in case of error
			OCI_Rollback(m_Connection);
		}
		else if (m_type == OCI_TYPE::OCI_CXX_API)
		{
			m_conn->Rollback();
		}
		
		return nullptr;
	};
	
	
	TMyOracleResultSet* result_set = nullptr;

	OCI_MutexAcquire(m_mutex);
	result_set = FetchRecords(query);
	OCI_MutexRelease(m_mutex);

	return result_set;
}