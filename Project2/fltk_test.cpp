#include <iostream>
#include <string>
#include <cstring>
#include<FL/Fl.h>
#include<FL/Fl_Box.h>
#include<FL/Fl_Window.h>
#include<FL/Fl_Input.H>
#include<FL/Fl_Output.H>
#include<FL/Fl_Button.H>
#include <mysqlx/xdevapi.h>
//Console will prompt you for your server user name and password
//gui login user is "user" and password is "pass"
using namespace std;

string promptUser()
{
	string x;
	cout << "Enter server username:  " << endl;
	cin >> x;
	return x;
}
string promptPass()
{
	string x;
	cout << "Enter server password:  " << endl;
	cin >> x;
	return x;
}
mysqlx::Session mySession(mysqlx::SessionOption::HOST, "localhost",
	mysqlx::SessionOption::PORT, 33060,
	mysqlx::SessionOption::USER, promptUser(),
	mysqlx::SessionOption::PWD, promptPass());
mysqlx::Schema hotel = mySession.getSchema("hotel");
mysqlx::Table login = hotel.getTable("login");
mysqlx::Table loginAttempt = hotel.getTable("loginAttempt");
mysqlx::Table guest = hotel.getTable("guest");
int loginWindow();
int userMenuWindow();
int userWindow();
Fl_Window* win = new Fl_Window(750, 500, "Login");

void connectToServer()
{
	//lists all databases on accessed server
	list<mysqlx::Schema> schemaList = mySession.getSchemas();
	for (mysqlx::Schema schema : schemaList)
	{
		cout << schema.getName() << endl;
	}
	//After testing server connection login window opens
	loginWindow();
}
//Custom struct used to keep track of our text input fields and the text inside
struct LoginInfo
{
	//These are our login and create account buttons
	Fl_Button *Login;
	Fl_Button *CreateAcc;
	

	//These are our text boxes
	Fl_Box *welcome;
	Fl_Box *invalid;
	Fl_Box *loggedIn;
	//These are our two text input fields
	Fl_Input *Uname;
	Fl_Input *Pass;

	//These are the two string values of whatever text is in Uname and Pass currently
	//note that these are currently only going to be updated when the Login button callback is called
	char user[20];
	char pass[20];
};
struct UserMenu
{
	Fl_Button *rooms;

	Fl_Box *welcome;
};
UserMenu userMenu;
struct UserPage
{
	Fl_Button *getNamesByRoom;

	Fl_Box *getNamesByRoomText;
	Fl_Box *searchResultsText;

	Fl_Input *roomNum;

	char roomNumber[3];
	
};
UserPage userPage;
void login_cb(Fl_Widget* Login, void* p)
{
	//Here, we are recasting the void parameter p that we passed in back into a LoginInfo object
	LoginInfo* attempt = reinterpret_cast<LoginInfo*>(p);
	//copies the string values from each input field into our string variables in LoginInfo attempt
	strcpy_s(attempt->user, attempt->Uname->value());
	strcpy_s(attempt->pass, attempt->Pass->value());
	//function to loginAttempt
	 //insert
	string singleQuote = "'";
	string userGuess = attempt->user;
	string pwGuess = attempt->pass;

	//delete everything from loginAttempt
	loginAttempt.remove().execute();
	loginAttempt.insert("username", "password").values(userGuess, pwGuess).execute();
	//select
	mysqlx::RowResult loginAttemptResults = loginAttempt.select("username", "password").execute();
	mysqlx::Row loginAttemptRow = loginAttemptResults.fetchOne();

	//function to check loginAttempt against login
	string loginQuery = "username = '" + userGuess + singleQuote;
	mysqlx::RowResult loginResults = login.select("username", "password")
		.where(loginQuery).execute();
	mysqlx::Row loginRow = loginResults.fetchOne();
	//TODO crashes if username is not valid
	 //need to first check if the username is
	 //a valid username before running loginQuery
	mysqlx::string logAU = loginAttemptRow[0];
	mysqlx::string logAPW = loginAttemptRow[1];
	mysqlx::string logU = loginRow[0];
	mysqlx::string logPW = loginRow[1];
	std::cout << attempt->user << std::endl << attempt->pass << std::endl;
	if (logAU == logU && logAPW == logPW)
	{
		cout << "Login Success" << endl;
		win->hide();
		userMenuWindow();
		//userWindow();
	}
	else
	{
		cout << "Login Failed" << endl;
	}
}
void menu_cb(Fl_Widget*, void* p)
{
	userWindow();
}
void createAcc_cb(Fl_Widget*, void*)
{
	std::cout << "Sorry. This feature is not yet available" << std::endl;
}

static void searchLogByRoom_cb(Fl_Widget* getNamesByRoom, void* p)
{
	UserPage* userPage = reinterpret_cast<UserPage*>(p);
	strcpy_s(userPage->roomNumber, userPage->roomNum->value());
	string x = userPage->roomNumber;
	cout << x << endl;
	string singleQuote = "'";
	string comma = ", ";
	string closeParen = ")";
	string query = "roomNum = '" + x + singleQuote;
	mysqlx::RowResult myResult = guest.select("name", "roomNum").where(query).execute();
	mysqlx::Row row = myResult.fetchOne();
	mysqlx::string mysqlComma = ", ";

	//convert mysqlx::string to std::string to c string to fit label() parameter :)
	mysqlx::string searchResults;
	string finalSearchResults;
	char cstr[30];
	searchResults = row[0];
	finalSearchResults = searchResults;
	strcpy_s(cstr, finalSearchResults.c_str());
	cout << "cstr " << cstr << endl;
	cout << "fsr " << finalSearchResults << endl;
	userPage->searchResultsText->label(cstr);

}


int loginWindow()
{
	LoginInfo attempt; // create an empty instance of our custom struct defined earlier
	//Fl_Window* win = new Fl_Window(750, 500, "Login");
	win->begin();
	attempt.welcome = new Fl_Box(250, 0, 200, 200, "Welcome! Please log in below");
	attempt.invalid = new Fl_Box(250, 0, 200, 200, "Invalid Login, please try again");
	attempt.loggedIn = new Fl_Box(250, 0, 200, 200, "Successfully Logged in, Welcome!");
	attempt.loggedIn->hide();
	attempt.invalid->hide();
	attempt.Login = new Fl_Button(300, 300, 50, 25, "Login");
	attempt.Login->callback(login_cb, &attempt);
	attempt.CreateAcc = new Fl_Button(400, 300, 115, 25, "Create Account");
	attempt.CreateAcc->callback(createAcc_cb);
	attempt.Uname = new Fl_Input(300, 200, 215, 25, "Username:");
	//Uname.callback(Uname_cb);
	attempt.Pass = new Fl_Input(300, 250, 215, 25, "Password");
	//Pass.callback(Pass_cb);
	win->end();
	win->show();
	return Fl::run();
}
int userMenuWindow()
{
	//TODO Remove menu window
	//Menu window remains open behind the newly called window.
	Fl_Window *menu = new Fl_Window(750, 500, "Menu");

	menu->begin();

	userMenu.rooms = new Fl_Button(400, 200, 50, 25, "Rooms");
	userMenu.welcome = new Fl_Box(250, 0, 200, 200, "Welcome to the user menu");
	menu->hide();
	userMenu.rooms->callback(menu_cb, &userMenu);
	
	menu->end();
	menu->show();

	return Fl::run();
}
int userWindow()
{


	Fl_Window *win = new Fl_Window(750, 500, "User Page");
	win->begin();
	userPage.getNamesByRoomText = new Fl_Box(250, 0, 200, 200, "Get Names by Room Number");
	userPage.getNamesByRoom = new Fl_Button(300, 200, 50, 25, "OK");
	userPage.roomNum = new Fl_Input(360, 125, 50, 25, "roomNumber");

	userPage.getNamesByRoom->callback(searchLogByRoom_cb, &userPage);

	userPage.searchResultsText = new Fl_Box(250, 300, 200, 200);

	win->end();
	win->show();
	return Fl::run();
}
int main()
{
	connectToServer();
}
