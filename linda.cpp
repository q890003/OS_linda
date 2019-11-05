#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <omp.h>
using namespace std;

//other tread check tuple when they us read/in

//every time create thread, need to give them ID, better create ID from bottom to up.  class for each client with method.
int check_cmd(stringstream &);
void do_operation(stringstream &);
void write2tuple_space(string);
void match_tuple(string, string);   //(input read/in, input fields)


//int ClientID[1000];
vector<vector<string>> tuple_space;
vector<string> instruction;
bool want_exit = 0;
class client{
    int ID;  //maybe put ID here.
    string data_buffer;
};

int main(){
    cout << "Welcome to Linda!" << endl;
    cout << "command format is [client ID] [operation] [field(s)]" << endl;
    omp_set_num_threads(4);
    //#pragma omp parallel
    //{
		cout << " thread: "<< omp_get_thread_num() << endl;
		while(true){
			string cmd;
			string test;
			vector<string> Lin;
			cout << "$: ";
			getline(cin, cmd);
			stringstream sscmd(cmd);

			if(check_cmd(sscmd)){
				sscmd.clear();
				sscmd.seekg(0, sscmd.beg);
			   if(want_exit){
				break;
			   }
			   do_operation(sscmd);
			}
		}
	
    //}

    return 0;
}
void do_operation(stringstream &cmd_line){
    string ID;
    string operation;
    string argument_line;

    cmd_line >> ID;
    cmd_line >> operation;
    getline(cmd_line,argument_line);
    cmd_line.str("");
    cmd_line.clear();
   //check_ID_list();               If never see the ID, create for it.


    if(operation == "out")
        write2tuple_space(argument_line);

    if(operation == "read" || operation == "in"){
        match_tuple(operation, argument_line);
    }
    cout << "tuple space is " << tuple_space.size() <<endl;
}

int check_cmd(stringstream &cmd_line){
    string ID_check;
    string operation_check;
    string exit_cmd;
    //bool format_error = 0;


    //check ID
    if(cmd_line >> ID_check){		// if use "!cmd_line.eof()", it wont trigger at the begining. so as an whitespace at the end.
        exit_cmd = ID_check;
    }else{
        cout << "need Client ID" << endl;   
		return 0;
    }
    if(exit_cmd.compare("exit") == 0){
        want_exit = 1;
        return 1;       //go back to main.
    }
    for(int i=0; i< ID_check.length(); i++){
        if(!isdigit(ID_check[i])){
            cout << "client ID should be a number" << endl;
            return 0;
        }
    }

    //check operation
    if(!cmd_line.eof()){
        cmd_line >> operation_check;
    }else{
        cout << "need operation" << endl;
        return 0;
    }
    if( !( (operation_check=="in") || (operation_check=="out") || (operation_check=="read") )) {  //if it's neither three predefined operation
        cout << "operation should be either \"in\", \"read\",or \"out\"" << endl;
        return 0;
    }

    //check fields
    if(cmd_line.eof()){
        cout << "need field(s)" << endl;
        return 0;
    }
    return 1;
}
void write2tuple_space(string argument_line){
    vector<string> field_vector;
    string field_tok;
    stringstream parser(argument_line);
    while(parser >> field_tok){             //if use !ss.eof() , when there is whitespace at the end, it will run extra loop
        field_vector.push_back(field_tok);
    }
    tuple_space.push_back(field_vector);
        //cout <<"tuple space " << tuple_space.size() << endl;
}
void match_tuple(string operation, string argument_line){
    //initialization
    int in_flag = 0; //in_flag: 1 means in operation ; 0 means read operation.
    if(operation == "in"){
        in_flag = 1;
    }
    cout << "operation is " << operation <<endl;

    //get Linda_template
    stringstream parser(argument_line);
    vector<string> Linda_template;
    string field_tok;
    while(parser >> field_tok){
        Linda_template.push_back(field_tok);
    }

    //matching
    vector<string> field_vector;
    vector<vector<string>>::iterator iter_tuple_space;
    vector<string>::iterator iter_field_vector;
    for(iter_tuple_space = tuple_space.begin(); iter_tuple_space< tuple_space.end(); ++iter_tuple_space){

        field_vector = *iter_tuple_space;
        if(Linda_template.size() != field_vector.size())
            continue;

        int match_success = 0, i=0;
        for(iter_field_vector = field_vector.begin(), i = 0; iter_field_vector< field_vector.end(); ++iter_field_vector, ++i){
            if(Linda_template[i] == *iter_field_vector)
                match_success ++;
        }
        if(match_success == Linda_template.size()){
            if(!in_flag){   //it's read operation.
                cout << " it match for read operation!" << endl;
            }else if(in_flag){   //it's in operation.
                tuple_space.erase(iter_tuple_space);
                cout << "delete the tuple in tuple space" << endl;
            }
        }
    }
}
