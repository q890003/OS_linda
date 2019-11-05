#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <omp.h>
using namespace std;

//every time create thread, need to give them ID, better create ID from bottom to up. 
//class for each client with method.
int check_cmd(stringstream &);
int do_operation(stringstream &);  // return true means success ; return false means fail.
	void write2tuple_space(string);
	bool match_tuple(string, string);   //(input read/in, input fields)
	
class client{
	public:
		string ID;  
		string instruction_buffer;		
		bool lock = false;
		int client_fd;
		//can use map to preserve clients private value.
};


//-----global variable------
//int ClientID[1000];
vector<vector<string>> tuple_space;
vector<client> client_wait_vector;
pthread_mutex_t gLock;
bool want_exit = false;
int NUM_THREADS = 0;
//-----------------------------



int main(){
	
	//intialization
	string master_thread_cmd;
    cout << "Welcome to Linda!" << endl;
	cout << "How many clients?" << endl;
	getline(cin, master_thread_cmd);
	NUM_THREADS = atoi(master_thread_cmd.c_str());
	omp_set_num_threads(NUM_THREADS);
	client client_info[NUM_THREADS];

	//pthread_mutex_init(&gLock, NULL);
    cout << "command format is [client ID] [operation] [field(s)]" << endl;
    #pragma omp parallel
    {

			// master thread handle the cmd. and the cmd line should be stack in order.
			//when client get the response, it has to put the data to it's txt by itself.
			//if there is no data, client got to wait!!!!! local mutex?
			//master  listening.
			#pragma omp master
			{
				string cmd;
				stringstream sscmd;
				bool instruction_flag = false;
				vector<client>::iterator iter_client_wait_vector;
				
				cout << "master is listening" << endl;
				while(true){
					cout << "$: ";
					getline(cin, cmd);
					sscmd.str(cmd);
					
					if(check_cmd(sscmd)){				//check instruction.
						sscmd.clear();							//reuse the same data in stringstream ssmd.
						sscmd.seekg(0, sscmd.beg);
						if(want_exit == true)	break;
						if(!( do_operation(sscmd) ) ){		//if do operation fail. no corresponding tuple.
							sscmd.clear();							//reuse the same data in stringstream ssmd.
							sscmd.seekg(0, sscmd.beg);
							int instruction_ID = 0;
							sscmd >> instruction_ID;
							client_info[instruction_ID].instruction_buffer = cmd;
							client_info[instruction_ID].lock = true;			//true means it's locked.					
							client_wait_vector.push_back(client_info[instruction_ID]); //store current instruction.
						}
						sscmd.str("");
						sscmd.clear();
						cout <<"tuple space, size is " << tuple_space.size() << endl;
							
						//when currnet instruction success,
						//check if it helps other clients or not.
						for(iter_client_wait_vector = client_wait_vector.begin(); iter_client_wait_vector < client_wait_vector.end(); ++iter_client_wait_vector){
							cout <<"it is" << (*iter_client_wait_vector).instruction_buffer << endl;
							sscmd.str( (*iter_client_wait_vector).instruction_buffer );
							//if there is data, do it and pop out from instr. queue. otherwise, hold the thread.
							if ( do_operation(sscmd) == true ){
								client_wait_vector.erase(iter_client_wait_vector);
								
							}//else{
								
							//}
							sscmd.clear();							//reuse stringstream ssmd.
							sscmd.seekg(0, sscmd.beg);
						}
					}
				}
			}		
			//clients thread.
			//while(true){				
			//seems not able to share "cin" together.!!!!!!!!!!!!!!!!!!
			//seems not able to share "cin" together.!!!!!!!!!!!!!!!!!!
			//seems not able to share "cin" together.!!!!!!!!!!!!!!!!!!
			//seems not able to share "cin" together.!!!!!!!!!!!!!!!!!!
			//seems not able to share "cin" together.!!!!!!!!!!!!!!!!!!
			//if exit, release the threads!
			// when thread got data, write it to it's txt. otherwise wait.
			//	cout << " thread: "<< omp_get_thread_num() << "$: "  << endl;
				//client_wait_vector.push_back(cmd);	//threads put cmd to the share memory
			//}
		
    }
    return 0;
}
int check_cmd(stringstream &cmd_line){

	string ID_check;
    string operation_check;
    string exit_cmd;
	
    //check ID
    if(cmd_line >> ID_check){		// if use "!cmd_line.eof()", it wont trigger at the begining. so as an whitespace at the end.
        exit_cmd = ID_check;
		if(exit_cmd.compare("exit") == 0){
			want_exit = 1;
			return 1;       //go back to main.
		}
    }else{
        cout << "need Client ID" << endl;   
		return 0;
    }

    for(int i=0; i< ID_check.length(); i++){
        if(!isdigit(ID_check[i])){
            cout << "client ID should be a integer" << endl;
            return 0;
        }
    }
	int client_num_limit_check = atoi(ID_check.c_str());
	if( client_num_limit_check > NUM_THREADS){
		cout << "client ID exceeded" << endl;
        return 0;
	}

    //check operation
    if(cmd_line >> operation_check){
        //cmd_line >> operation_check;
    }else{
        cout << "need operation" << endl;
        return 0;
    }
    if( !( (operation_check=="in") || (operation_check=="out") || (operation_check=="read") )) {  //if it's neither three predefined operation
        cout << "operation should be either \"in\", \"read\",or \"out\"" << endl;
        return 0;
    }

    //check fields
    if(!(cmd_line >> operation_check)){
        cout << "need field(s)" << endl;
        return 0;
    }
    return 1;
}
int do_operation(stringstream &cmd_line){
    string client_ID;
    string operation;
    string argument_line;
	bool oeration_flag = false;
    cmd_line >> client_ID;
    cmd_line >> operation;
    getline(cmd_line,argument_line);

    if(operation == "out"){
        write2tuple_space(argument_line);
		return true;
	}
    if(operation == "read" || operation == "in"){
        oeration_flag = match_tuple(operation, argument_line);
		//if there is no match tuple, 
		//1. hold the client thread.
		return oeration_flag;
		//return false. let master thread put instruction to instruction queue.
	}
}
void write2tuple_space(string argument_line){
    vector<string> field_vector;
    string field_tok;
    stringstream parser(argument_line);
	
    while(parser >> field_tok){             //if use !ss.eof() , when there is whitespace at the end, it will run extra loop
        field_vector.push_back(field_tok);
    }
    tuple_space.push_back(field_vector);
}
bool match_tuple(string operation, string argument_line){
    //initialization
    int in_flag = 0; //in_flag: 1 means in operation ; 0 means read operation.
    if(operation == "in"){
        in_flag = 1;
    }

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
        field_vector = *iter_tuple_space;		//get tuple from tuple_space
        if(Linda_template.size() != field_vector.size()) continue;//if size not right, skip it.
            
        int match_success = 0, i=0;
        for(iter_field_vector = field_vector.begin(), i = 0; iter_field_vector< field_vector.end(); ++iter_field_vector, ++i){
            if(Linda_template[i] == *iter_field_vector)
                match_success ++;
        }
        if(match_success == Linda_template.size()){
            if(!in_flag){   //it's read operation.
                cout << " it match for read operation!" << endl;
				return true;
            }else if(in_flag){   //it's in operation.
                tuple_space.erase(iter_tuple_space);
                cout << "delete the tuple in tuple space" << endl;
				return true;
            }
        }
    }
	return false;
}
