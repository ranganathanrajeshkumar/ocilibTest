// -----------------------------------------------------------------------------
#ifndef __TMYORACLE_H__
#define __TMYORACLE_H__
// -----------------------------------------------------------------------------
#include "ocilib.hpp"
#include "TMyOracleResultSet.h"
// -----------------------------------------------------------------------------
using namespace ocilib;
// -----------------------------------------------------------------------------

class TMyOracle
{
public:
	TMyOracle();
	virtual ~TMyOracle();
	bool Connect(const std::string& user, const std::string& password, const std::string& db);
	void Disconnect();
	bool IsConnected() const;
	OCI_Connection* GetConnection() const;

	TMyOracleResultSet* ExecuteQuery(const std::string& query);

private:
	OCI_Connection* m_Connection;
};
// -----------------------------------------------------------------------------
#endif
// -----------------------------------------------------------------------------