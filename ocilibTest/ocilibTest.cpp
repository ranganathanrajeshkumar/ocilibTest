// -----------------------------------------------------------------------------
#include "ocilib.hpp"
#include "TMyOracle.h"
#include "TMyOracleResultSet.h"
#include "SqlConnection.h"
#include <thread>
// -----------------------------------------------------------------------------
static std::unique_ptr<SqlConnection> g_sql_conn = nullptr;
static auto g_oci_type = OCI_TYPE::OCI_C_API;

// -----------------------------------------------------------------------------

class Employee
{
public:

	explicit Employee(TMyOracle* sql, int id) 
		: sql(sql), m_id(id), m_dept_id(0), m_first_name(""), m_last_name(""), m_dob(""), m_address(""), m_department("")
    {}
	
    ~Employee() = default;

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

		const std::string query = "SELECT FIRSTNAME, LASTNAME, DOB, ADDRESS, DEPT_ID, DEPT_DESC FROM employee e INNER JOIN department d ON d.id = e.dept_id WHERE e.id =  " + id;

        // Execute a query
        std::unique_ptr<TMyOracleResultSet> rs(sql->ExecuteQuery(query));
        if (!rs || !rs->Rows())
        {
            std::cerr << "[WARN] Employee::Build(): Unable to execute the query" << std::endl;
            return false;
        }

		// Fetch the result
		m_first_name    = rs->Get("FIRSTNAME");
		m_last_name     = rs->Get("LASTNAME");
		m_dob           = rs->Get("DOB");
		m_address       = rs->Get("ADDRESS");   
		m_dept_id       = std::atoi(rs->Get("DEPT_ID").c_str());
		m_department    = rs->Get("DEPT_DESC");

        return true;
	}

	std::string ToString() const 
    {
        std::ostringstream out;
		out << std::setw(10) << m_id            << " | "
			<< std::setw(15) << m_first_name    << " | "
			<< std::setw(15) << m_last_name     << " | "			
			<< std::setw(30) << m_department;
        
		return out.str();
	}

private:
    int m_id;
    int m_dept_id;
    std::string m_first_name;
    std::string m_last_name;
    std::string m_dob;
    std::string m_address;
    std::string m_department;

    TMyOracle* sql;
};
 // -----------------------------------------------------------------------------
int ocitest(TMyOracle* sql)
{   
    if (!sql)
    {
        std::cerr << "[ERROR] ocitest: Failed to get SQL connection" << std::endl;
        return EXIT_FAILURE;
    }
	if (!sql->IsConnected())
	{
		std::cerr << "[ERROR] ocitest: SQL connection is not connected" << std::endl;
		return EXIT_FAILURE;
	}
	// Get the number of loops from the user
	//std::cout << "Enter the number of loops: ";
	int loops = 100;
	//std::cin >> loops;
	
    auto tries = 0;
    while (tries++ < loops)
    {
        std::vector<std::unique_ptr<Employee>> employees;
        for (int i = 1; i <= 1000; ++i)
        {                                   
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

        employees.clear();
    }
    	
	return EXIT_SUCCESS;
}

int main(int argc, const char* argv[])
{
    if (g_oci_type == OCI_TYPE::OCI_CXX_API)
    {
        ocilib::Environment::Initialize(Environment::Default | Environment::Threaded );
        ocilib::Environment::EnableWarnings(true);
    }

    auto res = 0;
	try
	{
        // Initialize sql connection
        g_sql_conn = std::make_unique<SqlConnection>("dev", "123456", "orclpdb", g_oci_type);

        // Build the connection pool
        if (!g_sql_conn->Build())
        {
            std::cerr << "[ERROR] Main: Failed to build the SQL connection pool" << std::endl;
            return EXIT_FAILURE;
        }

        std::vector<std::thread> oci_test_threads;
		for (int i = 0; i < 20; ++i)
		{
            auto sql = g_sql_conn->GetConnection();
			oci_test_threads.emplace_back(ocitest, sql);
		}
		for (auto& thread : oci_test_threads)
		{
			if (thread.joinable())
			{
				thread.join();
			}
		}
		std::cout << "All threads completed successfully." << std::endl;

        g_sql_conn->Disconnect();
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}

    // Cleanup OCI CXX API
    if (g_oci_type == OCI_TYPE::OCI_CXX_API)
    {
        ocilib::Environment::Cleanup();
    }
	return res;
}

/*
int main(void)
{
    try
    {
        Environment::Initialize();

        Connection con("orclpdb", "dev", "123456");
        con.SetAutoCommit(true);
        //con.SetStatementCacheSize(10);

        // Get the number of loops from the user
        std::cout << "Enter the number of loops: ";
        int loops = 0;
        std::cin >> loops;
        if (loops <= 0)
        {
            std::cerr << "[ERROR] Main: Invalid number of loops" << std::endl;
            return EXIT_FAILURE;
        }

        auto tries = 0;
        while (tries++ < loops)
        {
            for (int i = 0; i < 1000; ++i)
            {
                Statement st(con);

                const std::string query = "SELECT FIRSTNAME, LASTNAME, DOB, ADDRESS, DEPT_ID, DEPT_DESC FROM employee e INNER JOIN department d ON d.id = e.dept_id WHERE e.id =  " + std::to_string(i + 1);
                st.Execute(query);

                Resultset rs = st.GetResultset();

                while (!rs.IsNull() && rs.Next())
                {
                    std::cout << std::setw(10) << i+1 << " | "
                        << std::setw(20) << rs.Get<ostring>(1) << " " << rs.Get<ostring>(2) << " | "
                        << std::setw(20) << rs.Get<ostring>(6) <<  " |\r\n"
                        << std::endl;
                }

            }
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