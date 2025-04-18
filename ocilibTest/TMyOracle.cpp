// -----------------------------------------------------------------------------
#include "TMyOracle.h"	
#include "utils.h"
// -----------------------------------------------------------------------------

TMyOracle::TMyOracle()
	: m_Connection{nullptr}
{
	
}

// -----------------------------------------------------------------------------
TMyOracle::~TMyOracle()
{
	Disconnect();
	OCI_Cleanup();
}
// -----------------------------------------------------------------------------

bool TMyOracle::Connect(const std::string& user, const std::string& password, const std::string& db)
{
	// Disconnect if already connected
	Disconnect();

	try
	{
		// Initialize OCI
		if (!OCI_Initialize(NULL, NULL, OCI_ENV_THREADED | OCI_ENV_CONTEXT))
		{
			std::cerr << "Failed to initialize OCI" << std::endl;
			return false;
		}

		// Create a new OCI environment
		m_Connection = OCI_ConnectionCreate(db.c_str(), user.c_str(), password.c_str(), OCI_SESSION_DEFAULT);
		if (!m_Connection)
		{
			std::cerr << "Failed to connect to database: " << OCI_ErrorGetString(OCI_GetLastError()) << std::endl;
			return false;
		}
				
		// Set auto-commit mode
		OCI_SetAutoCommit(m_Connection, true);

		// Set the statement cache size
		OCI_SetStatementCacheSize(m_Connection, 10);
		
		std::cout << "Connected to database: " << db << std::endl;

		return true;

	}	
	catch (const std::exception& ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;		
	}

	return false;
	
}
// -----------------------------------------------------------------------------
void TMyOracle::Disconnect()
{
	if (m_Connection)
	{
		OCI_ConnectionFree(m_Connection);
		m_Connection = nullptr;
	}
}
// -----------------------------------------------------------------------------
bool TMyOracle::IsConnected() const
{
	if (m_Connection)
	{
		return OCI_IsConnected(m_Connection);
	}

	return false;
}
// -----------------------------------------------------------------------------
OCI_Connection* TMyOracle::GetConnection() const
{
	return m_Connection;
}
// -----------------------------------------------------------------------------
TMyOracleResultSet* TMyOracle::ExecuteQuery(const std::string& query)
{
	try
	{
		if (!m_Connection)
		{
			std::cerr << "Not connected to database" << std::endl;
			return nullptr;
		}
		// Create a new statement
		OCI_Statement* stmt = OCI_StatementCreate(m_Connection);
		if (!stmt)
		{
			std::cerr << "Failed to create statement" << std::endl;
			return nullptr;
		}
		// Prepare and execute the statement
		if (!OCI_Prepare(stmt, query.c_str()))
		{
			std::cerr << "Failed to prepare statement: " << OCI_ErrorGetString(OCI_GetLastError()) << std::endl;
			OCI_StatementFree(stmt);
			return nullptr;
		}
		// Execute the statement
		if (!OCI_Execute(stmt))
		{
			std::cerr << "Failed to execute statement: " << OCI_ErrorGetString(OCI_GetLastError()) << std::endl;
			OCI_StatementFree(stmt);
			return nullptr;
		}

		// Commit the transaction
		OCI_Commit(m_Connection);

		// Get the result set
		TMyOracleResultSet* result_set = TMyOracleResultSet::ExtractResultSet(OCI_GetResultset(stmt));
		
		// Check if the result set is valid and return it
		if (result_set)
		{
			// Free the statement			
			OCI_StatementFree(stmt);
			return result_set;
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;
	}

	// Rollback in case of error
	OCI_Rollback(m_Connection);

	return nullptr;
}