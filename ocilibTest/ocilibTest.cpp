// -----------------------------------------------------------------------------
#include "ocilib.hpp"
#include "TMyOracle.h"
// -----------------------------------------------------------------------------
using namespace ocilib;
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
	
    void Disconnect()
    {
        for (auto& sql : m_sqls)
        {
            if (sql)
            {
                sql->Disconnect();
            }
        }
    }


public:
    bool Build()
	{
        try
        {
            for (int i = 0; i < m_max_connections; ++i)
            {
                std::unique_ptr<TMyOracle> sql(new TMyOracle());
                if (!sql->Connect(m_user, m_password, m_db))
                {
                    throw std::exception("Failed to connect to database");
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

	TMyOracle* GetConnection()
	{
		if (m_sqls.empty())
		{
			std::cerr << "[WARN] SqlConnection::GetConnection(): No available connections" << std::endl;
			return nullptr;
		}
		m_curr_conn_index = (m_curr_conn_index + 1) % m_sqls.size();
		return m_sqls[m_curr_conn_index].get();
	}

	
private:
	std::vector<std::unique_ptr<TMyOracle>> m_sqls;
	
    int m_max_connections = 10;
	int m_curr_conn_index = 0;

	std::string m_user;
	std::string m_password;
	std::string m_db;

};

class Employee
{
public:
	int m_id;
	std::string m_first_name;
	std::string m_last_name;
	std::string m_dob;
	std::string m_address;

	explicit Employee(TMyOracle* sql, int id) 
		: sql(sql), m_id(id), m_first_name(""), m_last_name(""), m_dob(""), m_address("")
    {}

	
	bool Build() 
	{
		if (!sql)
		{
			std::cerr << "[ERROR] Employee::Build(): SQL connection is null" << std::endl;
			return false;
		}

		if (m_id <= 0)
		{
			std::cerr << "[ERROR] Employee::Build(): Invalid emoloyee id" << std::endl;
			return false;
		}

        std::string id = std::to_string(m_id);

        // Execute a query
        std::unique_ptr<TMyOracleResultSet> rs(sql->ExecuteQuery("SELECT * FROM Employee WHERE ID=" + id));
        if (!rs || !rs->Rows())
        {
            std::cerr << "[ERROR] Employee::Build(): Failed to execute query" << std::endl;
            return false;
        }

		// Fetch the result
		m_first_name = rs->Get("FIRSTNAME");
		m_last_name = rs->Get("LASTNAME");
		m_dob = rs->Get("DOB");
		m_address = rs->Get("ADDRESS");        

        return true;
	}

	std::string ToString() const 
    {
        std::ostringstream out;
		out << std::setw(10) << m_id            << " | "
			<< std::setw(15) << m_first_name    << " | "
			<< std::setw(15) << m_last_name     << " | "
			<< std::setw(15) << m_dob           << " | "
			<< std::setw(30) << m_address;
        
		return out.str();
	}

private:
    TMyOracle* sql;
};


SqlConnection g_sql_conn("dev", "123456", "orclpdb");

int main(int argc, const char* argv[])
{
    int tries = 0;

	// Initialize OCI
    if(!g_sql_conn.Build())
	{
		std::cerr << "[ERROR] Main: Failed to initialize OCI" << std::endl;
		return EXIT_FAILURE;
	}
		
    std::vector<std::unique_ptr<Employee>> employees;
	for (int i = 1; i <= 1000; ++i)
	{
		// Get a connection
        auto sql = g_sql_conn.GetConnection();
        if (!sql)
        {
            std::cerr << "[ERROR] Main: Failed to get SQL connection" << std::endl;
            return EXIT_FAILURE;
        }
    
		// Create an employee object
		auto emp = std::make_unique<Employee>(sql, i);
		if (!emp->Build())
		{
			std::cerr << "[WARN] Main: Failed to build employee for id=" + std::to_string(i) << std::endl;
			continue;
		}
		employees.push_back(std::move(emp));
	}

	for (const auto& emp : employees)
	{
		std::cout << emp->ToString() << std::endl;
	}
	
	return EXIT_SUCCESS;
}

/*
int main(void)
{
    try
    {
        Environment::Initialize();

        Connection con("orclpdb", "dev", "123456");
        con.SetAutoCommit(true);

        Statement st(con);
        
        st.Execute("select * from Family");

        Resultset rs = st.GetResultset();

        while (!rs.IsNull() && rs.Next())
        {
            std::cout << std::setw(10) << rs.Get<ostring>(1) << " | "
                      << std::setw(20) << rs.Get<ostring>(2) << " | "
                      << std::setw(20) << rs.Get<ostring>(3) << " | "
                      << std::setw(11) << rs.Get<ostring>(4) << " | "
                      << std::setw(48) << rs.Get<ostring>(5) << " | "
                      << std::setw(20) << rs.Get<ostring>(6) << "\r\n"
                      << std::endl;
        }


        //st.Prepare("INSERT INTO NAME (FIRSTNAME, LASTNAME, PHONE, ID_NEW) VALUES ('Samyukta', 'RajeshKumar', '1212121212', '5' ) RETURNING ID_NEW INTO :code");
    
        //st.Register<int>(":code");
        
        //st.ExecutePrepared();

        //rs = st.GetResultset();
        //while (rs++)
        //{
        //    std::cout << "Item with ID = " << rs.Get<int>(1) << " has been inserted " << std::endl;
        //}

        //std::cout << "=> Total updated rows : " << rs.GetCount() << std::endl;


        //st.Prepare("UPDATE NAME SET PHONE=:lpn WHERE FIRSTNAME='Rajesh' RETURNING ID_NEW INTO :code");

        //st.Register<int>(":code");

        //ostring ph;
        //st.Bind(":lph", ph, BindInfo::In);
        //ph = "1234567890";

        //st.ExecutePrepared();

        //rs = st.GetResultset();
        //while (rs++)
        //{
        //    std::cout << "Item with ID = " << rs.Get<int>(1) << " has been inserted " << std::endl;
        //}

        //std::cout << "=> Total updated rows : " << rs.GetCount() << std::endl;        
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }

    Environment::Cleanup();

    return EXIT_SUCCESS;
}

/*

#include <iostream>

#include "ocilib.hpp"

using namespace ocilib;

int main(void)
{
    try
    {
        Environment::Initialize();

        Connection con("localhost:1521/XE", "SYSTEM", "123456");
        
        Statement st(con);
                
        st.Prepare("UPDATE NAME SET PHONE=9844751523 WHERE FIRSTNAME='Ramya'  RETURNING ID_NEW INTO :code");                
       
        st.Register<int>(":code");

        st.ExecutePrepared();
                
        auto rs = st.GetResultset();
               
        while (!rs.IsNull() && rs++)
        {
            std::cout << "Item with ID = " << rs.Get<int>(1) << " has been inserted " << std::endl;
        }

        std::cout << "=> Total updated rows : " << rs.GetCount() << std::endl;

        con.Commit();
        
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }

    Environment::Cleanup();

    return EXIT_SUCCESS;
}*/