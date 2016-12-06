#define PROJECTNAME "docat"
#define PROJECTVER PROJECTNAME ## _versions
#define USEMSV_GENERALCPP

#define USEMSV_HASH

#include "../../opensource/msvcore2/msvcore.cpp"

Versions PROJECTVER[]={
	// new version to up
	"0.0.0.3", "06.12.2016 02:05",
	"0.0.0.2", "05.12.2016 22:19",
	"0.0.0.1", "02.12.2016 14:23"
};

#include "docat.h"

int main(int args, char* arg[]){
	msvcoremain(args, arg);
	print(PROJECTNAME, " v.", PROJECTVER[0].ver, " (", PROJECTVER[0].date, ").\r\n");

	// Get args
	ILink &link = msvcorestate.link;

	// Get first option
	VString path = link.GetArg();

	// Change dir
	if(path)
		chdir(SString(path));

	int ret = DoCat();

	if(!ret)
		int error = 1;

	return !ret;
}