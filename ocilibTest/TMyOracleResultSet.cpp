//----------------------------------------------------------------------------
#include "TMyOracleResultSet.h"
//----------------------------------------------------------------------------
void TMyOracleResultSet::AddRow(const std::vector<std::string>& row) 
{
    m_rows.emplace_back(row);
}
//----------------------------------------------------------------------------
TMyOracleResultSet* TMyOracleResultSet::ExtractResultSet(OCI_Resultset* rs)
{
	if (!rs)
	{
		std::cerr << "[ERROR] TMyOracleResultSet::ExtractResultSet: Resultset is null" << std::endl;
		return nullptr;
	}

	// Create a new result set object
    TMyOracleResultSet* resultSet = new TMyOracleResultSet();

    while (OCI_FetchNext(rs)) 
    {
        std::vector<std::string> row;
        const unsigned int colCount = OCI_GetColumnCount(rs);
        for (unsigned int i = 1; i <= colCount; ++i)
        {
            OCI_Column* col = OCI_GetColumn(rs, i);
        
            auto column = OCI_GetColumn(rs, i);
            const auto type = OCI_ColumnGetType(col);
            const auto name = OCI_ColumnGetName(col);
			resultSet->AddColumn(std::to_upper(name));

            if (OCI_IsNull(rs, i)) 
            {
                row.emplace_back("");
                continue;
            }

            switch (type)
            {           
            case OCI_CDT_DATETIME: 
            {
                OCI_Date* dt = OCI_GetDate(rs, i);

				constexpr size_t SIZE_STR = 260;
                std::array<otext, SIZE_STR> str{};

				OCI_DateToText(dt, "YYYY-MM-DD HH24:MI:SS", SIZE_STR, str.data());                
                row.emplace_back(str.data());

                break;
            }
            default:
                row.emplace_back(std::string(OCI_GetString(rs, i)));
                break;
            }
        }

        resultSet->AddRow(row);
    }

    return resultSet;
}
//----------------------------------------------------------------------------
TMyOracleResultSet* TMyOracleResultSet::ExtractResultSet(ocilib::Resultset* rs)
{
	if (!rs)
	{
		std::cerr << "[ERROR] TMyOracleResultSet::ExtractResultSet: Resultset is null" << std::endl;
		return nullptr;
	}
	if (rs->IsNull())
	{
		std::cerr << "[ERROR] TMyOracleResultSet::ExtractResultSet: Resultset is empty" << std::endl;
		return nullptr;
	}

	TMyOracleResultSet* resultSet = new TMyOracleResultSet();
	while (rs->Next())
	{
		std::vector<std::string> row;
		const unsigned int colCount = rs->GetColumnCount();
		for (unsigned int i = 1; i <= colCount; ++i)
		{
			const auto col = rs->GetColumn(i);
			const auto name = col.GetName();
			const auto type = col.GetType();
            resultSet->AddColumn(std::to_upper(name));


			if (type == OCI_CDT_DATETIME)
			{
				ocilib::Date dt = rs->Get<ocilib::Date>(i);
				std::string dateStr = dt.ToString("YYYY-MM-DD HH24:MI:SS");
				row.emplace_back(dateStr);				
			}
            else
            {
                rs->IsNull() ? row.emplace_back("") : row.emplace_back(rs->Get<std::string>(i));
            }			
		}
		resultSet->AddRow(row);
	}
	return resultSet;	    
}
//----------------------------------------------------------------------------