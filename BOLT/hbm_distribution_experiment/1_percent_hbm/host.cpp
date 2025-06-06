#include <iostream>
#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>
#include <xrt/xrt_uuid.h>
#include <chrono>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <chrono>
#include <numeric>
#include <vector>
#include <xrt/xrt_bo.h>
#include <iomanip>
#include <vector>
#include <cstring> // For memset
#include <xrt/xrt_bo.h>
#include <CL/cl_ext_xilinx.h>
#define CMD_GET 0x01
#define CMD_PUT 0x02
#define RESP_ACK 0x10
#define RESP_ERR 0x11
#define CMD_SIZE 5000
#define VALUE_SIZE 8   // bytes 
#define PAGE_SIZE 2097152
#define PAGE_CAPACITY 8
#define HBM_CAPACITY 16
#define HASH_SIZE 3145728
#define EMPTY_KEY -1
#define INIT_SIZE 1000000
#define QUEUE_SIZE 10000000
using namespace std;

struct KV_TUPLE {
    uint32_t key;
    unsigned char value[VALUE_SIZE]; 
};

struct Value{
    unsigned char value[VALUE_SIZE];
};

struct  Page
{
    /* data */
    KV_TUPLE tuples[PAGE_CAPACITY];
};


struct Command_Packet{
    uint8_t type; //command type
    KV_TUPLE tuple; //key-value pair
};


struct Response_Packet{
    uint8_t type; //response type
    unsigned char value[VALUE_SIZE]; 
};


struct Attribute{
    uint32_t key;
    uint8_t  location_type;
    uint32_t page_index; //It is allocated to which page. For kv_tuples inside the host memory or eviction buffer
    uint32_t location_index; // If it is inside the host_mem, then this is its index inside the page. Or if it is inside the eviction buffer, then this is its index inside the eviction buffer for the page_index entry. Otherwise, it is the index inside the hbm bank.
};

struct Eviction_Buffer_Block{
    uint32_t key;
    uint32_t value_index;
};

static int bucket[HASH_SIZE];
static int head=0;
static int tail=0;
static std::unordered_set<uint32_t>insert_error_keys;
static int record[CMD_SIZE];// used to record the command number -> index in tuple for test correctness
static int get_cmd_count=0;
static int put_cmd_count=0;
static int hbm_cmd_count=0;
static int host_cmd_count=0;
static std::vector <uint32_t>hbm_sets; //key sets for hbm
static std::vector <uint32_t>host_sets; // key sets for the host memory
static std::unordered_map<uint32_t, int> hbm_key_to_index;  // Maps HBM keys to their indices in init_tuples
static std::unordered_map<uint32_t, int> host_key_to_index; // Maps host keys to their indices in init_tuples




void print_Command_Packet(Command_Packet packet){
     // Interpret the raw data
    uint8_t type = packet.type; // Assuming the first byte is the type
    printf("The type is 0x%02X\n", type);
    // Assuming key is stored in the next bytes, e.g., 1-4 (4 bytes key)
    uint32_t key = packet.tuple.key;
    printf("The key is %d\n", key);
    char value0=packet.tuple.value[0];
    printf("The value[0] is %d\n", value0);
}
void print_Response_Packet(Response_Packet packet){
     // Interpret the raw data
    uint8_t type = packet.type; // Assuming the first byte is the type
    printf("The type is 0x%02X\n", type);
    // Assuming key is stored in the next bytes, e.g., 1-4 (4 bytes key)
    char value0=packet.value[0];
    printf("The value[0] is %d\n", value0);
}


void print_Tuple(KV_TUPLE tuple){
    printf("The key is %ld and the value is %d\n",tuple.key,tuple.value[0]);
}

std::vector<xrt::bo> generate_bos_host_key_value_seperate_complete(xrt::device &device,xrt::kernel &init_kernel){
    std::vector<xrt::bo>bos;
    xrt::bo position_map_bo= xrt::bo(device,HASH_SIZE*HBM_CAPACITY*sizeof(Attribute), init_kernel.group_id(0));
    bos.push_back(position_map_bo);
    xrt::bo value_bo= xrt::bo(device,INIT_SIZE* sizeof(Value), init_kernel.group_id(1));
    bos.push_back(value_bo);
    xrt::bo queue_bo=xrt::bo(device,sizeof(uint32_t)*QUEUE_SIZE,init_kernel.group_id(2));
    bos.push_back(queue_bo);
    xrt::bo page_tuple_count_bo=xrt::bo(device,PAGE_SIZE*sizeof(uint32_t),init_kernel.group_id(3));
    bos.push_back(page_tuple_count_bo);
    xrt::bo host_page_bo=xrt::bo(device,PAGE_SIZE*sizeof(Page),xrt::bo::flags::host_only,init_kernel.group_id(4));
    bos.push_back(host_page_bo);
    xrt::bo eviction_buffer_bo=xrt::bo(device,PAGE_CAPACITY*PAGE_SIZE*sizeof(Eviction_Buffer_Block),init_kernel.group_id(5));
    bos.push_back(eviction_buffer_bo);
    printf("Finish to generate bos\n");
    return bos;
}


std::vector<KV_TUPLE> generate_distributed_init_tuples(uint32_t size) {
    std::vector<KV_TUPLE> tuples;
    tuples.reserve(size); // Reserve space to avoid reallocation during insertion
    uint32_t seed=16;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<uint32_t> key_dis(0, 4294967295); // For generating random keys
    std::uniform_int_distribution<uint8_t> value_dis(0, 255); // For generating random values
    std::unordered_set<uint32_t> used_keys;

    while(tuples.size()<size){    
        KV_TUPLE tuple;
        uint32_t key=key_dis(gen);
        if(used_keys.insert(key).second){
            tuple.key=key;
            for(int i=0;i<VALUE_SIZE;i++){
                tuple.value[i]=value_dis(gen);
            }
            tuples.push_back(tuple);
        }
    }
    return tuples;
}

std::vector<Command_Packet> generate_accuracy_test_commands(std::vector<KV_TUPLE>init_tuples){
    std::vector<Command_Packet> commands;
    for(int i=0;i<init_tuples.size();i++){
        Command_Packet packet;
        packet.type=CMD_PUT;
        packet.tuple=init_tuples[i];
        commands.push_back(packet);
    }
    for(int i=0;i<init_tuples.size();i++){
        Command_Packet packet;
        packet.type=CMD_GET;
        packet.tuple=init_tuples[i];
        commands.push_back(packet);
    }
    return commands;
}


std::vector<Command_Packet> generate_uniform_commands(int size, std::vector<KV_TUPLE> init_tuples) {
    std::vector<Command_Packet> commands;
    uint32_t seed = 1;
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> op_dis(0.0, 1.0); // Decide read or write type
    std::uniform_int_distribution<int> key_dis(0, init_tuples.size() - 1); // Select from all keys
    std::uniform_int_distribution<uint8_t> value_dis(0, 255); // For generating random values
    
    double write_ratio = 0.5;
    get_cmd_count = 0;
    put_cmd_count = 0;
    
    for (int i = 0; i < size; i++) {
        Command_Packet packet;
        double read_write_op = op_dis(gen);
        int key_index = key_dis(gen);
        
        packet.tuple.key = init_tuples[key_index].key;
        record[i] = key_index; // Record which tuple this command refers to
        
        if (read_write_op < write_ratio) {
            packet.type = CMD_PUT;
            
            // Generate random value for PUT command
            for (int j = 0; j < VALUE_SIZE; j++) {
                packet.tuple.value[j] = value_dis(gen);
            }
            
            ++put_cmd_count;
            
            // Track if it's HBM or host memory
            if (hbm_key_to_index.find(packet.tuple.key) != hbm_key_to_index.end()) {
                hbm_cmd_count++;
            } else if (host_key_to_index.find(packet.tuple.key) != host_key_to_index.end()) {
                host_cmd_count++;
            }
        } else {
            packet.type = CMD_GET;
            
            ++get_cmd_count;
            
            // Track if it's HBM or host memory
            if (hbm_key_to_index.find(packet.tuple.key) != hbm_key_to_index.end()) {
                hbm_cmd_count++;
            } else if (host_key_to_index.find(packet.tuple.key) != host_key_to_index.end()) {
                host_cmd_count++;
            }
        }
        
        commands.push_back(packet);
    }
    
    std::cout << "Generated " << commands.size() << " uniform commands:" << std::endl;
    std::cout << "  GET commands: " << get_cmd_count << " (" << (100.0 * get_cmd_count / commands.size()) << "%)" << std::endl;
    std::cout << "  PUT commands: " << put_cmd_count << " (" << (100.0 * put_cmd_count / commands.size()) << "%)" << std::endl;
    std::cout << "  HBM access: " << hbm_cmd_count << " (" << (100.0 * hbm_cmd_count / commands.size()) << "%)" << std::endl;
    std::cout << "  Host access: " << host_cmd_count << " (" << (100.0 * host_cmd_count / commands.size()) << "%)" << std::endl;
    
    return commands;
}


void run_accuracy_test(xrt::device &device, xrt::kernel &init_kernel,xrt::kernel &chain_kernel, std::vector<KV_TUPLE> init_tuples,std::vector<xrt::bo> bos) {
    // Allocate memory dynamically
    Attribute (*position_map)[HBM_CAPACITY] = new Attribute[HASH_SIZE][HBM_CAPACITY];
    Value* value = new Value[INIT_SIZE];
    uint8_t* bitmap = new uint8_t[INIT_SIZE];
    // Track number of real tuples in each page
    // Initialize all counts to 0
    uint32_t* page_tuple_count = new uint32_t[PAGE_SIZE]();
    uint32_t* queue = new uint32_t[QUEUE_SIZE];
    Page* pages = new Page[PAGE_SIZE];
    Eviction_Buffer_Block (*eviction_buffer)[PAGE_CAPACITY] = new Eviction_Buffer_Block[PAGE_SIZE][PAGE_CAPACITY];
    // KV_TUPLE (*eviction_buffer)[PAGE_CAPACITY] = new KV_TUPLE[PAGE_SIZE][PAGE_CAPACITY];
    
    auto init_start=std::chrono::system_clock::now();;
    // Initialize data structures with empty values
    for(int i = 0; i < HASH_SIZE; i++) {
        for(int j = 0; j < HBM_CAPACITY; j++) {
            position_map[i][j].key = EMPTY_KEY;
            position_map[i][j].location_type = EMPTY_KEY;
        }
    }
    //Initialize eviction buffer
    for(int i = 0; i < PAGE_SIZE; i++) {
        for(int j = 0; j < PAGE_CAPACITY; j++) {
            eviction_buffer[i][j].key = EMPTY_KEY;
        }
    }

    // Initialize bitmap, pages, and eviction buffer
    memset(bitmap, 0, INIT_SIZE * sizeof(uint8_t));
    for(int i = 0; i < PAGE_SIZE; i++) {
        for(int j = 0; j < PAGE_CAPACITY; j++) {
            pages[i].tuples[j].key = EMPTY_KEY;
            eviction_buffer[i][j].key = EMPTY_KEY;
        }
    }

    // Set up random number generation
    uint32_t seed = 1;
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> prob_dist(0.0, 1.0);
    std::uniform_int_distribution<> page_dist(0, PAGE_SIZE - 1);
    
    int free_index = 0;  // Points to next free index in HBM
    const int MAX_PAGE_RETRIES = 100;  // Maximum number of retries for finding a valid page

    // Process each tuple
    for(int i = 0; i < init_tuples.size(); i++) {
        const auto& tuple = init_tuples[i];
        double prob = prob_dist(gen);
        int hash_index = tuple.key % HASH_SIZE;
        bool inserted = false;

        if(prob < 0.01) {  //1% chance for HBM
            // Try to insert into HBM
            for(int j = 0; j < HBM_CAPACITY; j++) {
                if(position_map[hash_index][j].key == EMPTY_KEY) {
                    // Insert into HBM
                    position_map[hash_index][j].key = tuple.key;
                    position_map[hash_index][j].location_type = 0;
                    memcpy(&value[free_index].value, tuple.value, VALUE_SIZE);
                    position_map[hash_index][j].location_index = free_index;
                    bitmap[free_index] = 1;
                    free_index++;
                    inserted = true;
                    hbm_sets.push_back(tuple.key);
                    hbm_key_to_index[tuple.key] = i;  // Store mapping
                    break;
                }
            }
        } else {  // 99% chance for page
            // Try to find a suitable page randomly
            int retry_count = 0;
            while (retry_count < MAX_PAGE_RETRIES) {
                int page_idx = page_dist(gen);
                if(page_tuple_count[page_idx] < 4) {
                    // Find empty slot in the page
                    for(int j = 0; j < PAGE_CAPACITY; j++) {
                        if(pages[page_idx].tuples[j].key == EMPTY_KEY) {
                            // Insert into page
                            pages[page_idx].tuples[j] = tuple;
                            page_tuple_count[page_idx]++;
                            
                            // Update position map
                            for(int k = 0; k < HBM_CAPACITY; k++) {
                                if(position_map[hash_index][k].key == EMPTY_KEY) {
                                    position_map[hash_index][k].key = tuple.key;
                                    position_map[hash_index][k].location_type = 1;
                                    position_map[hash_index][k].page_index = page_idx;
                                    position_map[hash_index][k].location_index = j;
                                    inserted = true;
                                    host_sets.push_back(tuple.key);
                                    host_key_to_index[tuple.key] = i;  // Store mapping
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    if(inserted) break;
                }
                retry_count++;
            }
        }

        if(!inserted) {
            insert_error_keys.insert(tuple.key);
        }
    }

    // Initialize queue with remaining free indices
    tail = 0;
    for(int i = 0; i < INIT_SIZE; i++) {
        if(bitmap[i] == 0) {
            queue[tail++] = i;
        }
    }

    std::cout << "HBM count is " << hbm_sets.size() << std::endl;
    std::cout << "Host count is " << host_sets.size() << std::endl;
    std::cout << "Insert error count is " << insert_error_keys.size() << std::endl;
    std::cout << "The hbm_key_to_index size is " << hbm_key_to_index.size() << std::endl;
    std::cout << "The host_key_to_index size is " << host_key_to_index.size() << std::endl;



    auto position_map_bo_map=bos[0].map<Attribute*>();
    std::memcpy(position_map_bo_map,position_map,HASH_SIZE*HBM_CAPACITY*sizeof(Attribute));
    bos[0].sync(XCL_BO_SYNC_BO_TO_DEVICE);
    auto value_bo_map=bos[1].map<Value*>();
    std::memcpy(value_bo_map,value,INIT_SIZE*sizeof(Value));
    bos[1].sync(XCL_BO_SYNC_BO_TO_DEVICE);
    auto queue_bo_map=bos[2].map<uint32_t*>();
    std::memcpy(queue_bo_map,queue,QUEUE_SIZE*sizeof(uint32_t));
    bos[2].sync(XCL_BO_SYNC_BO_TO_DEVICE);
    auto page_tuple_count_bo_map=bos[3].map<uint32_t*>();
    std::memcpy(page_tuple_count_bo_map,page_tuple_count,PAGE_SIZE*sizeof(uint32_t));
    bos[3].sync(XCL_BO_SYNC_BO_TO_DEVICE);
    auto page_bo_map=bos[4].map<Page*>();
    std::memcpy(page_bo_map,pages,PAGE_SIZE*sizeof(Page));
    bos[4].sync(XCL_BO_SYNC_BO_TO_DEVICE);
    auto eviction_buffer_bo_map=bos[5].map<Eviction_Buffer_Block*>();
    std::memcpy(eviction_buffer_bo_map,eviction_buffer,PAGE_CAPACITY*PAGE_SIZE*sizeof(Eviction_Buffer_Block));
    bos[5].sync(XCL_BO_SYNC_BO_TO_DEVICE);
    auto init_end=std::chrono::system_clock::now();;
    std::chrono::duration<double> init_time=init_end-init_start;
    std::cout<<"The init time is "<<init_time.count()<<"s"<<std::endl;
    //Generate Commands
    const int command_size=CMD_SIZE;
    std::vector<Command_Packet> commands=generate_accuracy_test_commands(init_tuples);
    std::vector<Response_Packet> responses;
    //Allocate and initialize buffers
    xrt::bo cmd_bo=xrt::bo(device,commands.size()*sizeof(Command_Packet),chain_kernel.group_id(6));
    auto cmd_bo_map=cmd_bo.map<Command_Packet*>();
    std::memcpy(cmd_bo_map,commands.data(),commands.size()*sizeof(Command_Packet));
    cmd_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    xrt::bo resp_bo=xrt::bo(device,commands.size()*sizeof(Response_Packet),chain_kernel.group_id(7));
    auto start=std::chrono::system_clock::now();
    auto chain_kernel_run=chain_kernel(bos[0],bos[1],bos[2],bos[3],bos[4],bos[5],cmd_bo,resp_bo,commands.size(),head,tail);
    chain_kernel_run.wait();
    auto end=std::chrono::system_clock::now();
    double result=0;
    double kernel_time_in_sec=0;
    std::chrono::duration<double> kernel_time(0);
    kernel_time=std::chrono::duration<double>(end-start);
    kernel_time_in_sec=kernel_time.count();
    result=result/kernel_time_in_sec;
    std::cout<<"Command size is "<<commands.size()<<std::endl;
    double avg_latency=0;
    avg_latency=kernel_time_in_sec/(commands.size());
    std::cout<<"total time = "<<kernel_time_in_sec<<"s"<<std::endl;
    std::cout<<"AVERAGE LATENCY = "<<avg_latency<<"s"<<std::endl;
    resp_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    auto resp_bo_map=resp_bo.map<Response_Packet*>();
    int count=0;
    int not_insert_count=0;
    int mistake=0;

    //Test once
    for(int i=0;i<init_tuples.size();i++){
        if(resp_bo_map[i].type==RESP_ACK){
            //Compare the response to the init_tuples
            KV_TUPLE tuple=init_tuples[i];
            if(commands[i].type==CMD_GET){
                //If it is a get command, then the response value should be equal to the init_tuples value
                if(std::memcmp(resp_bo_map[i].value,tuple.value,VALUE_SIZE)==0){
                    count++;
                }
                else{
                    std::cout<<"The response value "<<i<<" is not equal to the init_tuples value"<<std::endl;
                    print_Tuple(tuple);
                    printf("The key is %ld,value inside response is %d\n",tuple.key,resp_bo_map[i].value[0]);
                    mistake++;
                }
            }
            else{
                count++;
            }
        }
        else{
            //During initialization, it is not inserted into the hbm or host page beacuse of the full capacity generated by seed
            if(insert_error_keys.find(init_tuples[record[i]].key)!=insert_error_keys.end()){
                not_insert_count++;
            }
        }
    }
    // Test twice
    for(int i=0;i<init_tuples.size();i++){
        if(resp_bo_map[i+INIT_SIZE].type==RESP_ACK){
            //Compare the response to the init_tuples
            KV_TUPLE tuple=init_tuples[i];
            if(commands[i].type==CMD_GET){
                //If it is a get command, then the response value should be equal to the init_tuples value
                if(std::memcmp(resp_bo_map[i].value,tuple.value,VALUE_SIZE)==0){
                    count++;
                }
                else{
                    std::cout<<"The response value "<<i<<" is not equal to the init_tuples value"<<std::endl;
                    print_Tuple(tuple);
                    printf("The key is %ld,value inside response is %d\n",tuple.key,resp_bo_map[i].value[0]);
                    mistake++;
                }
            }
            else{
                count++;
            }
        }
        else{
            //During initialization, it is not inserted into the hbm or host page beacuse of the full capacity generated by seed
            if(insert_error_keys.find(init_tuples[record[i]].key)!=insert_error_keys.end()){
                not_insert_count++;
            }
        }
    }
    std::cout<<"The number of response is "<<count<<std::endl;
    std::cout<<"The number of mistake is "<<mistake<<std::endl;
    std::cout<<"Error casused by not inserted into the hbm or host page (random seed) is "<<not_insert_count<<std::endl;
    std::cout<<"The number of get command is "<<get_cmd_count<<std::endl;
    std::cout<<"The number of put command is "<<put_cmd_count<<std::endl;
}

void run_init_and_chain_kernel_1_percent_hbm_with_probility(xrt::device &device, xrt::kernel &init_kernel,xrt::kernel &chain_kernel, std::vector<KV_TUPLE> init_tuples,std::vector<xrt::bo> bos) {
    // Allocate memory dynamically
    Attribute (*position_map)[HBM_CAPACITY] = new Attribute[HASH_SIZE][HBM_CAPACITY];
    Value* value = new Value[INIT_SIZE];
    uint8_t* bitmap = new uint8_t[INIT_SIZE];
    uint32_t* queue = new uint32_t[QUEUE_SIZE];
    Page* pages = new Page[PAGE_SIZE];
    Eviction_Buffer_Block (*eviction_buffer)[PAGE_CAPACITY] = new Eviction_Buffer_Block[PAGE_SIZE][PAGE_CAPACITY];
    // KV_TUPLE (*eviction_buffer)[PAGE_CAPACITY] = new KV_TUPLE[PAGE_SIZE][PAGE_CAPACITY];
    
    // Track number of real tuples in each page
    int* page_tuple_count = new int[PAGE_SIZE]();  // Initialize all counts to 0
    auto init_start=std::chrono::system_clock::now();;
    // Initialize data structures with empty values
    for(int i = 0; i < HASH_SIZE; i++) {
        for(int j = 0; j < HBM_CAPACITY; j++) {
            position_map[i][j].key = EMPTY_KEY;
            position_map[i][j].location_type = EMPTY_KEY;
        }
    }

    // Initialize bitmap, pages, and eviction buffer
    memset(bitmap, 0, INIT_SIZE * sizeof(uint8_t));
    for(int i = 0; i < PAGE_SIZE; i++) {
        for(int j = 0; j < PAGE_CAPACITY; j++) {
            pages[i].tuples[j].key = EMPTY_KEY;
            eviction_buffer[i][j].key = EMPTY_KEY;
        }
    }

    // Set up random number generation
    uint32_t seed = 1;
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> prob_dist(0.0, 1.0);
    std::uniform_int_distribution<> page_dist(0, PAGE_SIZE - 1);
    
    int free_index = 0;  // Points to next free index in HBM
    const int MAX_PAGE_RETRIES = 100;  // Maximum number of retries for finding a valid page

    // Process each tuple
    for(int i = 0; i < init_tuples.size(); i++) {
        const auto& tuple = init_tuples[i];
        double prob = prob_dist(gen);
        int hash_index = tuple.key % HASH_SIZE;
        bool inserted = false;

        if(prob < 0.01) {  // 1% chance for HBM
            // Try to insert into HBM
            for(int j = 0; j < HBM_CAPACITY; j++) {
                if(position_map[hash_index][j].key == EMPTY_KEY) {
                    // Insert into HBM
                    position_map[hash_index][j].key = tuple.key;
                    position_map[hash_index][j].location_type = 0;
                    memcpy(&value[free_index].value, tuple.value, VALUE_SIZE);
                    position_map[hash_index][j].location_index = free_index;
                    bitmap[free_index] = 1;
                    free_index++;
                    inserted = true;
                    hbm_sets.push_back(tuple.key);
                    hbm_key_to_index[tuple.key] = i;  // Store mapping
                    break;
                }
            }
        } else {  // 99% chance for page
            // Try to find a suitable page randomly
            int retry_count = 0;
            while (retry_count < MAX_PAGE_RETRIES) {
                int page_idx = page_dist(gen);
                if(page_tuple_count[page_idx] < 4) {
                    // Find empty slot in the page
                    for(int j = 0; j < PAGE_CAPACITY; j++) {
                        if(pages[page_idx].tuples[j].key == EMPTY_KEY) {
                            // Insert into page
                            pages[page_idx].tuples[j] = tuple;
                            page_tuple_count[page_idx]++;
                            
                            // Update position map
                            for(int k = 0; k < HBM_CAPACITY; k++) {
                                if(position_map[hash_index][k].key == EMPTY_KEY) {
                                    position_map[hash_index][k].key = tuple.key;
                                    position_map[hash_index][k].location_type = 1;
                                    position_map[hash_index][k].page_index = page_idx;
                                    position_map[hash_index][k].location_index = j;
                                    inserted = true;
                                    host_sets.push_back(tuple.key);
                                    host_key_to_index[tuple.key] = i;  // Store mapping
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    if(inserted) break;
                }
                retry_count++;
            }
        }

        if(!inserted) {
            insert_error_keys.insert(tuple.key);
        }
    }

    // Initialize queue with remaining free indices
    tail = 0;
    for(int i = 0; i < INIT_SIZE; i++) {
        if(bitmap[i] == 0) {
            queue[tail++] = i;
        }
    }

    std::cout << "HBM count is " << hbm_sets.size() << std::endl;
    std::cout << "Host count is " << host_sets.size() << std::endl;
    std::cout << "Insert error count is " << insert_error_keys.size() << std::endl;
    std::cout << "The hbm_key_to_index size is " << hbm_key_to_index.size() << std::endl;
    std::cout << "The host_key_to_index size is " << host_key_to_index.size() << std::endl;


    auto position_map_bo_map=bos[0].map<Attribute*>();
    std::memcpy(position_map_bo_map,position_map,HASH_SIZE*HBM_CAPACITY*sizeof(Attribute));
    bos[0].sync(XCL_BO_SYNC_BO_TO_DEVICE);
    auto value_bo_map=bos[1].map<Value*>();
    std::memcpy(value_bo_map,value,INIT_SIZE*sizeof(Value));
    bos[1].sync(XCL_BO_SYNC_BO_TO_DEVICE);
    auto queue_bo_map=bos[2].map<uint32_t*>();
    std::memcpy(queue_bo_map,queue,QUEUE_SIZE*sizeof(uint32_t));
    bos[2].sync(XCL_BO_SYNC_BO_TO_DEVICE);
    auto page_tuple_count_bo_map=bos[3].map<uint32_t*>();
    std::memcpy(page_tuple_count_bo_map,page_tuple_count,PAGE_SIZE*sizeof(uint32_t));
    bos[3].sync(XCL_BO_SYNC_BO_TO_DEVICE);
    auto page_bo_map=bos[4].map<Page*>();
    std::memcpy(page_bo_map,pages,PAGE_SIZE*sizeof(Page));
    bos[4].sync(XCL_BO_SYNC_BO_TO_DEVICE);
    auto eviction_buffer_bo_map=bos[5].map<Eviction_Buffer_Block*>();
    std::memcpy(eviction_buffer_bo_map,eviction_buffer,PAGE_CAPACITY*PAGE_SIZE*sizeof(Eviction_Buffer_Block));
    bos[5].sync(XCL_BO_SYNC_BO_TO_DEVICE);
    auto init_end=std::chrono::system_clock::now();;
    std::chrono::duration<double> init_time=init_end-init_start;
    std::cout<<"The init time is "<<init_time.count()<<"s"<<std::endl;
    //Generate Commands
    const int command_size=CMD_SIZE;
    std::vector<Command_Packet> commands=generate_uniform_commands(command_size,init_tuples);
    std::vector<Response_Packet> responses;
    //Allocate and initialize buffers
    xrt::bo cmd_bo=xrt::bo(device,commands.size()*sizeof(Command_Packet),chain_kernel.group_id(6));
    auto cmd_bo_map=cmd_bo.map<Command_Packet*>();
    std::memcpy(cmd_bo_map,commands.data(),commands.size()*sizeof(Command_Packet));
    cmd_bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    xrt::bo resp_bo=xrt::bo(device,commands.size()*sizeof(Response_Packet),chain_kernel.group_id(7));
    auto start=std::chrono::system_clock::now();
    auto chain_kernel_run=chain_kernel(bos[0],bos[1],bos[2],bos[3],bos[4],bos[5],cmd_bo,resp_bo,commands.size(),head,tail);
    chain_kernel_run.wait();
    auto end=std::chrono::system_clock::now();
    double result=0;
    double kernel_time_in_sec=0;
    std::chrono::duration<double> kernel_time(0);
    kernel_time=std::chrono::duration<double>(end-start);
    kernel_time_in_sec=kernel_time.count();
    result=result/kernel_time_in_sec;
    std::cout<<"Command size is "<<commands.size()<<std::endl;
    double avg_latency=0;
    avg_latency=kernel_time_in_sec/(commands.size());
    std::cout<<"total time = "<<kernel_time_in_sec<<"s"<<std::endl;
    std::cout<<"AVERAGE LATENCY = "<<avg_latency<<"s"<<std::endl;
    resp_bo.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    auto resp_bo_map=resp_bo.map<Response_Packet*>();
    int count=0;
    int not_insert_count=0;
    int mistake=0;
    bos[0].sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    // for(int i=0;i<CMD_SIZE;i++){
    //     if(resp_bo_map[i].type==RESP_ACK){
    //         //Compare the response to the init_tuples
    //         KV_TUPLE tuple=init_tuples[record[i]];
    //         if(commands[i].type==CMD_GET){
    //             //If it is a get command, then the response value should be equal to the init_tuples value
    //             if(std::memcmp(resp_bo_map[i].value,tuple.value,VALUE_SIZE)==0){
    //                 count++;
    //             }
    //             else{
    //                 std::cout<<"The response value "<<i<<" is not equal to the init_tuples value"<<std::endl;
    //                 print_Tuple(tuple);
    //                 printf("The key is %ld,value inside response is %d\n",tuple.key,resp_bo_map[i].value[0]);
    //                 mistake++;
    //             }
    //         }
    //         else{
    //             count++;
    //         }
    //         // int hash_index=tuple.key%HASH_SIZE;
    //         // Attribute* hash_position_map=position_map+hash_index*HBM_CAPACITY;
    //         // int empty_index=-1;
    //         // for(int j=0;j<HBM_CAPACITY;j++){
    //         //     if(hash_position_map[j].key==tuple.key){
    //         //         empty_index=j;
    //         //         break;
    //         //     }
    //         // }
    //         // printf("The key is %ld and the entry is %ld, and the index inside the entry is %ld and its location type is %ld, its location index is %ld\n",tuple.key,hash_index,empty_index,hash_position_map[empty_index].location_type,hash_position_map[empty_index].location_index);
    //     }
    //     else{
    //         //During initialization, it is not inserted into the hbm or host page beacuse of the full capacity generated by seed
    //         if(insert_error_keys.find(init_tuples[record[i]].key)!=insert_error_keys.end()){
    //             not_insert_count++;
    //         }
    //     }
    // }
    // std::cout<<"The number of response is "<<count<<std::endl;
    // std::cout<<"The number of mistake is "<<mistake<<std::endl;
    // std::cout<<"Error casused by not inserted into the hbm or host page (random seed) is "<<not_insert_count<<std::endl;
    // std::cout<<"The number of get command is "<<get_cmd_count<<std::endl;
    // std::cout<<"The number of put command is "<<put_cmd_count<<std::endl;
}


int main(int argc, char** argv) {
    system("sync && echo 3 | sudo tee /proc/sys/vm/drop_caches");
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <xclbin_file>" << std::endl;
        return EXIT_FAILURE;
    }

    const char* xclbin_filename = argv[1];
    // Step 1: Open the FPGA device
    auto device = xrt::device(0);  // Open the first available FPGA deviceHBM_CAPACITY
    auto uuid = device.load_xclbin(xclbin_filename);  // Load the bitstream
    // Step 2: Create the kernel object
    auto init_kernel= xrt::kernel(device, uuid, "init_kernel");
    auto chain_kernel= xrt::kernel(device, uuid, "chain_kernel");
    //test 80% hbm using one test function
    printf("Start testing 1% HBM\n");
    std::vector<KV_TUPLE> init_tuples=generate_distributed_init_tuples(INIT_SIZE);
    printf("Init tuples size is %ld\n",init_tuples.size());
    printf("The value size is %ld\n",VALUE_SIZE);
    auto bos=generate_bos_host_key_value_seperate_complete(device,init_kernel);
    run_init_and_chain_kernel_1_percent_hbm_with_probility(device,init_kernel,chain_kernel,init_tuples,bos);
    // run_accuracy_test(device,init_kernel,chain_kernel,init_tuples,bos);
    return 0;
}