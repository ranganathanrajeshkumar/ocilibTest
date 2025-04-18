//----------------------------------------------------------------------------
#include "TMyOracleResultSet.h"
//----------------------------------------------------------------------------
//TMyOracleResultSet::TMyBlob TMyOracleResultSet::ReadOCILob(OCI_Lob* lob)
//{
//    const auto size = static_cast<unsigned int>(OCI_LobGetLength(lob));
//    TMyBlob data(size);
//    OCI_LobRead(lob, data.data(), size);
//    return data;
//}
////----------------------------------------------------------------------------
//TMyOracleResultSet::TMyBlob TMyOracleResultSet::ReadOCIRaw(OCI_Resultset* rs, unsigned int colIndex)
//{
//    void* ptr = nullptr;
//    const auto size = 0;
//    OCI_GetRaw(rs, colIndex, ptr ,size);
//    if (ptr == nullptr || size == 0)
//        return {};
//
//    return TMyBlob((uint8_t*)ptr, (uint8_t*)ptr + size);
//}
////----------------------------------------------------------------------------
//std::string TMyOracleResultSet::ReadClob(OCI_Lob* lob)
//{
//    const auto char_count = static_cast<unsigned int>(OCI_LobGetLength(lob));
//    if (char_count == 0)
//        return "";
//
//    std::vector<char> buffer(char_count + 1);  // +1 for null terminator
//    unsigned int read_chars = OCI_LobRead(lob, buffer.data(), char_count);
//
//    return std::string(buffer.data(), read_chars);
//}
////----------------------------------------------------------------------------
//TMyOracleResultSet::TMyDateTime TMyOracleResultSet::ReadOCIDateTime(OCI_Date* dt)
//{
//	TMyOracleResultSet::TMyDateTime dateTime;
//	OCI_DateGetDate(dt, &dateTime.year, &dateTime.month, &dateTime.day);
//	OCI_DateGetTime(dt, &dateTime.hour, &dateTime.minute, &dateTime.second);
//	return dateTime;
//}
//----------------------------------------------------------------------------
void TMyOracleResultSet::AddRow(const std::vector<std::string>& row) 
{
    m_rows.emplace_back(row);
}
//----------------------------------------------------------------------------
TMyOracleResultSet* TMyOracleResultSet::ExtractResultSet(OCI_Resultset* rs)
{
    TMyOracleResultSet* resultSet = new TMyOracleResultSet();

    while (OCI_FetchNext(rs)) 
    {
        std::vector<std::string> row;
        const unsigned int colCount = OCI_GetColumnCount(rs);
        for (unsigned int i = 1; i <= colCount; ++i)
        {
            OCI_Column* col = OCI_GetColumn(rs, i);
        
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