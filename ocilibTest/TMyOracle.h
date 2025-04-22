// -----------------------------------------------------------------------------
#ifndef __TMYORACLE_H__
#define __TMYORACLE_H__
// -----------------------------------------------------------------------------
#include "ocilib.hpp"
#include <atomic>
#include <memory>
// -----------------------------------------------------------------------------
using namespace ocilib;
// -----------------------------------------------------------------------------
enum class OCI_TYPE
{
	OCI_C_API = 1,
	OCI_CXX_API = 2
};
// -----------------------------------------------------------------------------
class TMyOracle;
class TMyOracleStatement;
class TMyOracleResultSet;
// -----------------------------------------------------------------------------

class TMyOracleStatement
{
	OCI_Statement* stmt = nullptr;
public:
	explicit TMyOracleStatement(OCI_Connection* conn) : stmt(OCI_StatementCreate(conn)) {}
	~TMyOracleStatement() { if (stmt) OCI_StatementFree(stmt); }

	// Prevent copying
	TMyOracleStatement(const TMyOracleStatement&) = delete;
	TMyOracleStatement& operator=(const TMyOracleStatement&) = delete;

	// Allow moving
	TMyOracleStatement(TMyOracleStatement&& other) noexcept : stmt(other.stmt) { other.stmt = nullptr; }

	operator OCI_Statement* () const { return stmt; }
	
};

class TMyOracle
{
public:
	
	explicit TMyOracle(OCI_TYPE type = OCI_TYPE::OCI_C_API);
	virtual ~TMyOracle();
	bool Connect(const std::string& user, const std::string& password, const std::string& db);
	void Disconnect();
	bool IsConnected() const;

	template<typename T>
	T* GetConnection()
	{
		if (m_type == OCI_TYPE::OCI_CXX_API)
		{
			return m_conn.get();
		}
		else
		{
			return reinterpret_cast<T*>(m_Connection);
		}
	}

	int GetConnInstanceCounter() const { return m_conn_instance_counter; }

	std::string GetLastError() const { return m_lst_error; }
	std::string GetLastQuery() const { return m_lst_query; }

	TMyOracleResultSet* ExecuteQuery(const std::string& query);

private:
	std::string m_lst_query;
	std::string m_lst_error;
	OCI_TYPE m_type;
	ocilib::MutexHandle m_mutex;

	OCI_Connection* m_Connection;
	
	int m_conn_instance_counter{ 0 };

	std::unique_ptr<Connection> m_conn = nullptr;
};

// -----------------------------------------------------------------------------
#endif
// -----------------------------------------------------------------------------