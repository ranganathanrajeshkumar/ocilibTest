// -----------------------------------------------------------------------------
#ifndef __TMYORACLE_H__
#define __TMYORACLE_H__
// -----------------------------------------------------------------------------
#include "ocilib.hpp"
#include "TMyOracleResultSet.h"
#include <atomic>
// -----------------------------------------------------------------------------
using namespace ocilib;
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
	TMyOracle();
	virtual ~TMyOracle();
	bool Connect(const std::string& user, const std::string& password, const std::string& db);
	void Disconnect();
	bool IsConnected() const;
	OCI_Connection* GetConnection() const;
	std::string GetLastError() const { return m_lst_error; }
	std::string GetLastQuery() const { return m_lst_query; }
	TMyOracleResultSet* ExecuteQuery(const std::string& query);

private:
	std::string m_lst_query;
	std::string m_lst_error;

	static std::atomic<int> m_instanceCount;
	OCI_Connection* m_Connection;
};

// -----------------------------------------------------------------------------
#endif
// -----------------------------------------------------------------------------