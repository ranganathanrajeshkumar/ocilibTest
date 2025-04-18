// -----------------------------------------------------------------------------	
#include "SqlConnection.h"
// -----------------------------------------------------------------------------	
void SqlConnection::Disconnect()
{
	for (auto& sql : m_sqls)
	{
		if (sql)
		{
			sql->Disconnect();
		}
	}
}
// -----------------------------------------------------------------------------	
bool SqlConnection::Build()
{
	try
	{
		for (int i = 0; i < m_max_connections; ++i)
		{
			std::unique_ptr<TMyOracle> sql(new TMyOracle());
			if (!sql->Connect(m_user, m_password, m_db))
			{
				return false;
			}

			m_sqls.emplace_back(std::move(sql));
		}

		return true;
	}
	catch (const std::exception& ex)
	{
		std::cerr << "[EXCEPTION] SqlConnection::Build(): " << ex.what() << std::endl;
	}
	return false;
}
// -----------------------------------------------------------------------------
TMyOracle* SqlConnection::GetConnection()
{
	if (m_sqls.empty())
	{
		std::cerr << "[WARN] SqlConnection::GetConnection(): No available connections" << std::endl;
		return nullptr;
	}
	m_curr_conn_index = (m_curr_conn_index + 1) % m_sqls.size();
	return m_sqls[m_curr_conn_index].get();
}
// -----------------------------------------------------------------------------