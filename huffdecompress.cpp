#include<iostream> 
#include<unordered_map> 
using namespace std;

string dectobin(int decimal){
    string s="";
    for(int i = 7; i>=0; i--){
        if(decimal>>i & 1)s += '1';
        else s += '0';
    }
    return s;
}

int main(int argc, char *argv[]){
    if(argc != 2){
        cout<<"Invalid cmd line arg. Usage: ./a.out <input file>\n";
        return 1;
    }
    FILE *input = fopen(argv[1], "rb");
    int num_of_unique_chars=0;
    if(input == NULL){
        cout<<"Error opening input file\n";
        return 2;
    }
    string in = argv[1];
    string out = in.substr(0,in.find("-compressed")) + "-decompressed.";
    char buff[1];

    // Read file extension length and the extension itself
    fread(buff, 1, 1, input);
    int fsize = buff[0] - '0';
    while(fsize--){
        fread(buff, 1, 1, input);
        out += buff[0];
    }
    
    FILE *output = fopen(out.c_str(), "wb");
    if(output == NULL){
        cout<<"Error creating output file\n";
        return 3;
    }
    unordered_map<string, char> decode_map;
    string s = "";
    int flag=0;

    // Read and fill the hashmap to decode later
    while(fread(buff, 1, 1, input)){
        if(buff[0] != '\0'){
            flag = 0;
            s += buff[0];
        }
        else{
            num_of_unique_chars++;
            flag++;
            if(flag == 2)break;
            decode_map[s.substr(1, s.length()-1)] = s[0];
            s = "";
        }
    }

    // Read and store the padding, read extra null chars 
    fread(buff, 1, 1, input);
    int padding = buff[0] - '0';
    fread(buff, 1, 1, input);  // null
    fread(buff, 1, 1, input);  // null

    // Actual binary code begins from here.
    // Decode directly from bytes to avoid building a huge bit-string in memory.
    long data_start = ftell(input);
    fseek(input, 0, SEEK_END);
    long data_end = ftell(input);
    fseek(input, data_start, SEEK_SET);

    string current_bits = "";
    long bytes_processed = 0;
    while(fread(buff, 1, 1, input)){
        unsigned char byte = (unsigned char)buff[0];
        int bit_start = (bytes_processed == 0) ? padding : 0;

        for(int bit = bit_start; bit < 8; bit++){
            current_bits += ((byte >> (7 - bit)) & 1) ? '1' : '0';
            auto it = decode_map.find(current_bits);
            if(it != decode_map.end()){
                fwrite(&(it->second), 1, 1, output);
                current_bits = "";
            }
        }
        bytes_processed++;
        if(data_start + bytes_processed >= data_end){
            break;
        }
    }
    cout<<"Decompressed File Succesfully.\n";

    unordered_map<string, char>:: iterator iter;
    for(iter = decode_map.begin(); iter!=decode_map.end(); iter++){
        //cout<<iter->first<<"\t"<<iter->second<<"\n";
    }
    //cout<<"NUMBER OF UNIQUE CHARS - "<<num_of_unique_chars;
    fclose(input);
    fclose(output);
    return 0;
}
