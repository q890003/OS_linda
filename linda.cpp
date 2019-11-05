#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <omp.h>
using namespace std;
#define client_model 0
//every time create thread, need to give them ID, better create ID from bottom to up. 
//class for each client with method.
int check_cmd(stringstream &);
int first_operation(stringstream &);  // return true means success ; return false means fail.
	void write2tuple_space(string);
	bool match_tuple(stringstream &);  
	
class client{
	public:
		string ID;  
		string client_private_instruction_buffer;		
		bool lock = false;
		int client_fd;
		
		//can use map to preserve clients private value.
}client_info[1000];


//-----global variable------
//int ClientID[1000];
vector<vector<string>> tuple_space;
vector<client> client_order_record_vector;
//pthread_mutex_t gLock;
bool want_exit = false;
int NUM_THREADS = 0;
//-----------------------------



int main(){
	
	//-----------------intialization-------------------
	string server_cmd;
    cout << "Welcome to Linda!" << endl;
	cout << "How many clients?" << endl;
	getline(cin, server_cmd);
	NUM_THREADS = atoi(server_cmd.c_str());
	omp_set_num_threads(NUM_THREADS+1);			//client_info[0] is for cient model. 
	//global client client_info[NUM_THREADS+1];
	stringstream init;
	for(int i = 1; i <= NUM_THREADS ; i++){
		init << i;
		init >> client_info[i].ID;
		init.str("");
		init.clear();
	}
	//----------------------------------------------------
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

				cout << "master is listening" << endl;
				while(true){
					cout << "$: ";
					getline(cin, cmd);
					sscmd.str(cmd);
					
					if(check_cmd(sscmd)){				//check instruction.
						sscmd.clear();							//reuse the same data in stringstream ssmd.
						sscmd.seekg(0, sscmd.beg);
						if(want_exit == true)	break;
						if(first_operation(sscmd) ){		//tuple matched.
							cout << "got it" << endl;							
						}


						//when currnet instruction success, check if it helps other clients or not.
						bool wait_flag[NUM_THREADS+1];
						for(vector<client>::iterator iter_client_order_record_vector = client_order_record_vector.begin(); 
							iter_client_order_record_vector < client_order_record_vector.end();
							++iter_client_order_record_vector){
							
							//tag the first instruction of a client.
							if (wait_flag[ atoi(       (*iter_client_order_record_vector).ID.c_str()     )]  == false ){
								 wait_flag[atoi(        (*iter_client_order_record_vector).ID.c_str()    )] = true;
							}else{
								continue;
							}

							sscmd.str( (*iter_client_order_record_vector).client_private_instruction_buffer);								
							//if there is tuple for waiting client, take it and pop out. otherwise, hold the thread.
							if ( match_tuple(sscmd) == true ){		
								cout << "client[" << (*iter_client_order_record_vector).ID<< "] got"<<  (*iter_client_order_record_vector).client_private_instruction_buffer << endl;
								client_order_record_vector.erase(iter_client_order_record_vector);
							}
							
							sscmd.clear();							//reuse stringstream ssmd.
							sscmd.seekg(0, sscmd.beg);
						}
						
						cout <<"tuple space size: " << tuple_space.size() << endl;
						cout << " client_order_record_vector size: " << client_order_record_vector.size() << endl;
					}
				}
			}		
			//clients thread.
			//while(true){				
			//seems not able to share "cin" together.!!!!!!!!!!!!!!!!!!
			//seems not able to share "cin" together.!!!!!!!!!!!!!!!!!!
			//if exit, release the threads!
			// when thread got data, write it to it's txt. otherwise wait.
			//	cout << " thread: "<< omp_get_thread_num() << "$: "  << endl;
				//client_order_record_vector.push_back(cmd);	//threads put cmd to the share memory
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
int first_operation(stringstream &cmd_line){

    string client_ID;
    string operation;
    string argument_line;
	bool operation_flag = false;
		
    cmd_line >> client_ID;
    cmd_line >> operation;
    getline(cmd_line,argument_line);

    if(operation == "out"){
        write2tuple_space(argument_line);
		cmd_line.clear();							//reuse stringstream ssmd.
		cmd_line.seekg(0, cmd_line.beg);
		return true;
	}
    if(operation == "read" || operation == "in"){
		cmd_line.clear();							//reuse stringstream ssmd.
		cmd_line.seekg(0, cmd_line.beg);
        if( match_tuple(cmd_line) == true){
			//client_info[atoi(client_ID.c_str()].
			
			
		}else{	// no tuple for read/in
			string instruction;
			cmd_line.clear();							//reuse stringstream ssmd.
			cmd_line.seekg(0, cmd_line.beg);
			getline(cmd_line,instruction);
			client_info[client_model].ID= client_ID;	
			client_info[client_model].client_private_instruction_buffer= instruction;	
			client_order_record_vector.push_back(client_info[client_model]);
			cmd_line.clear();							//reuse stringstream ssmd.
			cmd_line.seekg(0, cmd_line.beg);
			return false;
			
		}
		//if there is no match tuple, 
		//1. hold the client thread.
		//return false. let master thread put instruction to instruction queue.
	}
	cmd_line.clear();							//reuse stringstream ssmd.
	cmd_line.seekg(0, cmd_line.beg);
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
bool match_tuple(stringstream &cmd_line){
    //initialization
    int in_flag = 0; //in_flag: 1 means in operation ; 0 means read operation.
	string client_ID;
    string operation;
    string argument_line;
	cmd_line >> client_ID;
    cmd_line >> operation;

    //get Linda_template
    vector<string> Linda_template;
    string field_tok;
    while(cmd_line >> field_tok){
        Linda_template.push_back(field_tok);
    }


    if(operation == "in"){	 //in_flag=1 means "in" operation.
        in_flag = 1;
    }
	//matching
	//searching tuple space
    vector<string> field_vector;
    for( vector<vector<string> >::iterator iter_tuple_space = tuple_space.begin(); 
		iter_tuple_space< tuple_space.end(); 
		++iter_tuple_space){
        field_vector = *iter_tuple_space;		//get tuple from tuple_space
        if(Linda_template.size() != field_vector.size()) continue;//if size not right, skip it.
		
        //comparing fields.
        int match_success = 0, i=0;
		vector<string>::iterator iter_field_vector;
        for(i = 0, iter_field_vector= field_vector.begin(); 
			iter_field_vector< field_vector.end(); 
			++iter_field_vector, ++i){
				
            if(Linda_template[i] == *iter_field_vector || Linda_template[i][0] =='?')
                match_success ++;
        }
		
        if(match_success == Linda_template.size()){
			client_info[atoi(client_ID.c_str() )].lock = false;
            if(!in_flag){   //it's read operation.
                cout << " it match for read operation!" << endl;
				return true;
            }else if(in_flag){   //it's in operation.
                tuple_space.erase(iter_tuple_space);
                cout << "delete the tuple in tuple space" << endl;
				for(i = 0, iter_field_vector= field_vector.begin(); 
					iter_field_vector< field_vector.end(); 
					++iter_field_vector, ++i){
						
					if(Linda_template[i][0] =='?')
						Linda_template[i] = *iter_field_vector;
				}
				for(int i = 0; i<Linda_template.size();i++){
					cout << "retrieve:  "<< Linda_template[i] << endl;
				}
				return true;
            }
        }
    }
	client_info[atoi(client_ID.c_str() )].lock = true;
	return false;
}
