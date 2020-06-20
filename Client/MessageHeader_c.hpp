enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_ERROR
};

struct DataHeader
{
	DataHeader() {
		DataLength = sizeof(DataHeader);
		cmd = CMD_ERROR;
	}
	short DataLength;
	short cmd;
};

//DataPackage
struct Login :public DataHeader
{
	Login()
	{
		DataLength = sizeof(Login);
		cmd = CMD_LOGIN;
		strcpy(Data, "abcdefg");
	}
	char UserName[32];
	char PassWord[32];
	char Data[32];
};

struct LoginResult :public DataHeader
{
	LoginResult()
	{
		DataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 3;
		strcpy(str, "abcdefg");
	}
	int result;
	char str[92];
};

struct NewUserJoin :public DataHeader
{
	NewUserJoin()
	{
		DataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};

struct Logout :public DataHeader
{
	Logout()
	{
		DataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char UserName[32];
};

struct LogoutResult :public DataHeader
{
	LogoutResult()
	{
		DataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 1;
		strcpy(Data, "avc");
	}
	int result;
	char Data[92];
};
