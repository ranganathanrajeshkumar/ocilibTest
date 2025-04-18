#ifndef __SQLCONNECTION_H__
#define __SQLCONNECTION_H__
// -----------------------------------------------------------------------------
#include "utils.h"
#include "TMyOracle.h"
// -----------------------------------------------------------------------------

class SqlConnection
{
public:
	explicit SqlConnection(const std::string& user, const std::string& password, const std::string& db)
		: m_user(user), m_password(password), m_db(db)
	{
	}

	~SqlConnection()
	{
		if (!m_sqls.empty())
		{
			Disconnect();
		}
	}

private:
	SqlConnection(const SqlConnection&) = delete;
	SqlConnection& operator=(const SqlConnection&) = delete;
	SqlConnection(SqlConnection&&) = delete;
	SqlConnection& operator=(SqlConnection&&) = delete;

	void Disconnect();

public:
	bool Build();

	TMyOracle* GetConnection();

private:
	std::vector<std::unique_ptr<TMyOracle>> m_sqls;

	int m_max_connections = 10;
	int m_curr_conn_index = 0;

	std::string m_user;
	std::string m_password;
	std::string m_db;

};

//------------------------------------------------------------------------------
#endif


