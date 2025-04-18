#ifndef __TMyOracleResultSetH__
#define __TMyOracleResultSetH__
// -----------------------------------------------------------------------------
#include "ocilib.hpp"
#include "utils.h"
// -----------------------------------------------------------------------------

// This class is a placeholder for the actual implementation of TMyOracleResultSet.
class TMyOracleResultSet
{
public:
    
    void AddRow(const std::vector<std::string>& row);
	      	
    const size_t Rows() const { return m_rows.size(); }	    

    const size_t Columns() const { return m_cols.size(); }

    void AddColumn(const std::string& colName) 
    { 
		if (GetColumnName(colName).empty())
		{
            m_cols.push_back(colName);
		}        
    }
    std::string GetColumnName(size_t index) const 
    {
        if (index <= m_cols.size())
        {
            return m_cols.at(index);
        }
        
		return "";
    }
    std::string GetColumnName(const std::string name) const 
    {
		for (const auto& it : m_cols)
		{
			if (it == name)
				return it;
		}

        return "";
    }

	const bool First()
    {
		if (m_rows.size() > 0)
		{
			m_currentRow = 0;
			return true;
		}
		return false;
	}

	const bool Next()
	{
		if (m_currentRow < m_rows.size())
		{
			m_currentRow++;
			return true;
		}
		return false;
	}   
    const bool Prev()
    {        
        if (m_currentRow > 0)
        {
			--m_currentRow;
			return true;
        }

		return false;
    }

	bool Eof() const
	{
		return m_currentRow >= m_rows.size();
	}

	std::string Get(size_t colIndex) const
	{		
		if (colIndex < m_cols.size())
		{
			if (m_currentRow < m_rows.size())
			{
				return m_rows.at(m_currentRow).at(colIndex);
			}			
		}
		return {};
	}

	std::string Get(const std::string& field_name) const
    {
        for (size_t i = 0; i < m_cols.size(); ++i)
        {
            if (m_cols[i] == std::to_upper(field_name))
            {
                return m_rows.at(m_currentRow).at(i);
            }
        }
        return {};
    }
               
    static TMyOracleResultSet* ExtractResultSet(OCI_Resultset* rs);

	std::vector<std::vector<std::string>> m_rows;
    std::vector<std::string> m_cols;
	size_t m_currentRow = 0;

};

// -----------------------------------------------------------------------------
#endif
// -----------------------------------------------------------------------------

