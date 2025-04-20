#ifndef __SQLCONNECTION_H__
#define __SQLCONNECTION_H__
// -----------------------------------------------------------------------------
#include "utils.h"
#include "TMyOracle.h"
// -----------------------------------------------------------------------------

class SqlConnection
{
public:
	explicit SqlConnection(const std::string& user, const std::string& password, const std::string& db, OCI_TYPE type = OCI_TYPE::OCI_C_API)
		: m_user(user), m_password(password), m_db(db), m_type(type)
	{
		if (m_type == OCI_TYPE::OCI_C_API)
		{
			// Initialize OCI C API
			OCI_Initialize(nullptr, nullptr, OCI_ENV_THREADED | OCI_ENV_CONTEXT);
			OCI_EnableWarnings(true);
		}
	}

	virtual ~SqlConnection()
	{	
		if (m_type == OCI_TYPE::OCI_C_API)
		{
			// Cleanup OCI C API
			OCI_Cleanup();
		}
	}

	void Disconnect();

private:
	SqlConnection(const SqlConnection&) = delete;
	SqlConnection& operator=(const SqlConnection&) = delete;
	SqlConnection(SqlConnection&&) = delete;
	SqlConnection& operator=(SqlConnection&&) = delete;
	

public:
	bool Build();
	bool IsConnected() const
	{
		return !m_sqls.empty();
	}
	TMyOracle* GetConnection();

private:
	std::vector<std::unique_ptr<TMyOracle>> m_sqls;

	int m_max_connections = 10;
	int m_curr_conn_index = 0;
	OCI_TYPE m_type;
	std::string m_user;
	std::string m_password;
	std::string m_db;


};

//------------------------------------------------------------------------------
#endif


