// codigo para la conexion entre ODBC A SQL SERVER : KARLA MILAGROS DOMINIGUEZ DOPORTO 12:40 AM 

#include <iostream>
#include <Windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>


using namespace std; 

void mostrarError(SQLHANDLE handle, SQLSMALLINT type) {
	SQLSMALLINT i = 0;
	SQLINTEGER native;
	SQLCHAR state[7];
	SQLCHAR text[256];
	SQLSMALLINT len;
	SQLRETURN ret;

	cout << "\n[ERROR ODBC]\n";
	while ((ret = SQLGetDiagRecA(type, handle, ++i, state, &native, text,
			256, &len)) == SQL_SUCCESS) {
		cout <<  "STATE: " << state <<  "-" << text << endl;
		}
}

//ESTO ES PARA MOSTRAR LOS ENCABEZADOS DE LAS TABLAS  02:55AM
void mostrarEncabezados(SQLHDBC dbc, const char* nombreTabla) {
	SQLHSTMT stmt;
	SQLRETURN ret;

	// CREAR LA INSTRUCCION
	ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
	if (!SQL_SUCCEEDED(ret)) {
		cout << "No se pudo crear el manejador para mostrar encabezados.\n";
		return;
	}

	// PARA OBTENER SOLO NOMBRES DE COLUMNAS
	string query = "SELECT TOP 0 * FROM ";
	query += nombreTabla;

	ret = SQLExecDirectA(stmt, (SQLCHAR*)query.c_str(), SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		cout << "No se pudo ejecutar SELECT para la tabla: " << nombreTabla << endl;
		SQLFreeHandle(SQL_HANDLE_STMT, stmt);
		return;
	}

	SQLSMALLINT numColumnas;
	SQLNumResultCols(stmt, &numColumnas);

	cout << "\nEncabezados de la tabla: " << nombreTabla << endl;
	cout << "-------------------------------------\n";

	//PARA MOSTRAR ENCABEZADOS 
	for (SQLUSMALLINT i = 1; i <= numColumnas; i++) {
		SQLCHAR nombreColumna[128];
		SQLSMALLINT longitudNombre;

		SQLDescribeColA(stmt, i, nombreColumna, sizeof(nombreColumna),
			&longitudNombre, NULL, NULL, NULL, NULL);

		cout << nombreColumna << " | ";
	}

	cout << "\n-------------------------------------\n";

	SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}


void crearTablas(SQLHDBC dbc) {
	SQLHSTMT stmt;
	SQLRETURN ret;

	ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
	if (!SQL_SUCCEEDED(ret)) {
		cout << "No se pudo crear el manejador para crear tablas.\n";
		return;
	}

	// Tabla CENTRO_TRABAJO
	string sqlCentroTrabajo =
		"IF NOT EXISTS (SELECT * FROM sys.objects WHERE name='CENTRO_TRABAJO' AND type='U') "
		"CREATE TABLE CENTRO_TRABAJO ("
		"idCentro INT PRIMARY KEY, "
		"nombre NVARCHAR(50), "
		"direccion NVARCHAR(100));";

	// Tabla EMPLEADO
	string sqlEmpleado =
		"IF NOT EXISTS (SELECT * FROM sys.objects WHERE name='EMPLEADO' AND type='U') "
		"CREATE TABLE EMPLEADO ("
		"idEmpleado INT PRIMARY KEY, "
		"nombre NVARCHAR(50), "
		"apellido NVARCHAR(50), "
		"idCentro INT FOREIGN KEY REFERENCES CENTRO_TRABAJO(idCentro));";

	// Tabla DIRECTIVO
	string sqlDirectivo =
		"IF NOT EXISTS (SELECT * FROM sys.objects WHERE name='DIRECTIVO' AND type='U') "
		"CREATE TABLE DIRECTIVO ("
		"idDirectivo INT PRIMARY KEY, "
		"nombre NVARCHAR(50), "
		"apellido NVARCHAR(50), "
		"idEmpleado INT FOREIGN KEY REFERENCES EMPLEADO(idEmpleado));";

	ret = SQLExecDirectA(stmt, (SQLCHAR*)sqlCentroTrabajo.c_str(), SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) mostrarError(stmt, SQL_HANDLE_STMT);

	ret = SQLExecDirectA(stmt, (SQLCHAR*)sqlEmpleado.c_str(), SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) mostrarError(stmt, SQL_HANDLE_STMT);

	ret = SQLExecDirectA(stmt, (SQLCHAR*)sqlDirectivo.c_str(), SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) mostrarError(stmt, SQL_HANDLE_STMT);

	SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

int main() {
	SQLHENV env;
	SQLHDBC dbc;
	SQLRETURN ret;


	// aqui se crea el entorno odbc

	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env) != SQL_SUCCESS) {
		cout << "no fue posible crear el entorno OBDC. \n";
		return 1;
	}

	SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

	// crear la conexion
	if (SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc) != SQL_SUCCESS) {
		cout << "no fue posible crear el manejador de conexion. \n";
		return 1;
	}
	cout << "Intentando conectar a SQL Server...?\n";

	// aqui es la cadena de conexion 01:12 am

	SQLCHAR connectionString[] =
		"DRIVER={ODBC Driver 17 for SQL Server};"
		"SERVER=DOMINGUEZ\\MSSQLSERVER01;"
		"DATABASE=EmpresaUNI;"
		"Trusted_Connection=yes;";
	ret = SQLDriverConnectA(
		dbc,
		NULL,
		connectionString,
		SQL_NTS,
		NULL,
		0,
		NULL,
		SQL_DRIVER_NOPROMPT

	);

	if (SQL_SUCCEEDED(ret)) {
		cout << "¡CONEXION EXITOSA ALA BASE DE DATOS EmpresaUNI!\n";
		// SOLO EN CASO DE QUE LA TABLA NO EXISTA ESTO SE EJECUTA
		crearTablas(dbc);
		
		
		// AQUI MOSTRAR ENCABEZADOS 03:12 AM 
		mostrarEncabezados(dbc, "CENTRO_TRABAJO");
		mostrarEncabezados(dbc, "EMPLEADO");
		mostrarEncabezados(dbc, "DIRECTIVO");
	}
	else {
		cout << "NO FUE POSIBLE CONECTAR A LA BASE DE DATOS.\n";
		mostrarError(dbc, SQL_HANDLE_DBC);
		return 1;
	}

	// cerrar conexion 01:25 am 
	SQLDisconnect(dbc);
	SQLFreeHandle(SQL_HANDLE_DBC, dbc);
	SQLFreeHandle(SQL_HANDLE_ENV, env);

	return 0;
}