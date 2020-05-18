#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gpio.h>
#include <ulfius.h>
#include <consts.h>
#include <rsa.h>
#include <search.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/evp.h>

#define PORT 8080
//unsigned char * _publicKeyBuffer;
unsigned char * _privateKeyBuffer;
struct _u_instance _serverInstance;
int _EXIT = 0;
FILE *_logFD= NULL;
//unsigned char  encrypted[4098]={};
//int encrypted_length;

struct ImageData {
    int size;
    unsigned char *buffer;
    int status;
};

/**
 * Reads an image into a buffer
 *
 * @param  pPath Path of the image
 * @return Buffer with the bytes read from the image
 */
struct ImageData readImage(char *pPath){
    FILE *file;
    unsigned char *buffer;
    unsigned long fileLen;
    struct ImageData data;

    //Open file
    file = fopen(pPath, "rb");
    if (!file) {
        data.status = 1; //Unable to open the image
        return;
    }

    //Get file length
    fseek(file, 0, SEEK_END);
    fileLen=ftell(file);
    data.size = ftell(file);
    fseek(file, 0, SEEK_SET);

    //Allocate memory
    buffer=(char *)malloc(fileLen);
    if (!buffer) {
        data.status = 2; //Error allocating buffer
        fclose(file);
        return;
    }

    fread(buffer, fileLen, sizeof(unsigned char), file);
    fclose(file);
    data.buffer = buffer;
    data.status = 0; //Success

    return data;
}

/**
 * Encodes binary data in base64 string
 *
 * @param  input  Buffer with binary data
 * @param  length Size of the buffer
 * @return        Base64 string
 */
char *base64(const unsigned char *input, int length) {
  const auto pl = 4*((length+2)/3);
  auto output = (char *)(calloc(pl+1, 1)); //+1 for the terminating null that EVP_EncodeBlock adds on
  const auto ol = EVP_EncodeBlock((unsigned char *)(output), input, length);
  //if (pl != ol) { std::cerr << "Whoops, encode predicted " << pl << " but we got " << ol << "\n"; }
  return output;
}

/**
 * Decodes string in base64 to binary data
 *
 * @param  input  String in base64
 * @param  length Length of the string
 * @return        Binary data
 */
unsigned char *decodeBase64(const char *input, int length) {
  const auto pl = 3*length/4;
  auto output = (unsigned char *)(calloc(pl+1, 1));
  const auto ol = EVP_DecodeBlock(output, (const unsigned char *)(input), length);
  //if (pl != ol) { std::cerr << "Whoops, decode predicted " << pl << " but we got " << ol << "\n"; }
  return output;
}

/**
 * Authenticates the user with a "username" and a password encrypted
 * with RSA.
 * The password is decrypted using the rsa library
 *
 * @param  pRequest  Http request received
 * @param  pResponse Response object
 * @param  pUserData User Data object
 * @return           json_t object
 */
int callback_login(const struct _u_request *pRequest, struct _u_response *pResponse, void *pUserData){

    char *user = u_map_get(pRequest->map_url, "user");
    char *pass = u_map_get(pRequest->map_header, "pass");

    fprintf(_logFD, "---> LOGIN REQUEST: user -> %s\n", user);
    fflush(_logFD);

    unsigned char decrypted[4098]={};
    //Checks if both user and pass have been provided
    if(user == NULL || pass == NULL) {
        ulfius_set_json_body_response(pResponse, 400, json_pack("{ss}", ERROR, LOGIN_ERR));
        return U_CALLBACK_COMPLETE;
    }

    if(strcmp(user, USER) == 0){
        unsigned char * base64Decoded = decodeBase64(pass, strlen(pass));
        private_decrypt(base64Decoded, ENCRYPTION_LENGTH, _privateKeyBuffer, decrypted);
        fprintf(_logFD, "Decrypted: %s\n", decrypted);
        fflush(_logFD);
        if(strcmp(decrypted, PASS) == 0){
            ulfius_set_json_body_response(pResponse, 200, json_pack("{ss}", OK, LOGIN_OK));
            fprintf(_logFD, "---> SUCCESSFUL LOGIN: %s\n", LOGIN_OK);
        }
        else{
            ulfius_set_json_body_response(pResponse, 401, json_pack("{ss}", ERROR, PASS_ERR));
            fprintf(_logFD, "---> LOGIN ERROR: %s\n", PASS_ERR);
        }
    }
    else{
        ulfius_set_json_body_response(pResponse, 404, json_pack("{ss}", ERROR, USER_ERR));
        fprintf(_logFD, "---> LOGIN ERROR: %s\n", USER_ERR);
    }

    fflush(_logFD);
    return U_CALLBACK_COMPLETE;
}

/**
 * Return the state of the door of the room specified in the URL
 *
 * @param  pRequest  Http request received
 * @param  pResponse Response object
 * @param  pUserData User Data object
 * @return           json_t object
 */
int callback_doors(const struct _u_request *pRequest, struct _u_response *pResponse, void *pUserData){

    int value = -1;
    char *location = u_map_get(pRequest->map_url, "location");
    if(location == NULL){
        ulfius_set_json_body_response(pResponse, 400, NULL);
        return U_CALLBACK_COMPLETE;
    }


    if(strcmp(location, "room1") == 0)
        value = digitalRead(DOORS[BEDROOM_1_DOOR]);
    else if(strcmp(location, "room2") == 0)
        value = digitalRead(DOORS[BEDROOM_2_DOOR]);
    else if(strcmp(location, "bath") == 0)
        value = digitalRead(DOORS[BATHROOM_DOOR]);
    else if(strcmp(location, "main") == 0)
        value = digitalRead(DOORS[MAIN_DOOR]);
    else if(strcmp(location, "garage") == 0)
        value = digitalRead(DOORS[GARAGE_DOOR]);
    else{
        ulfius_set_json_body_response(pResponse, 400, json_pack("{ss}", ERROR, ROOM_ERR));
        return U_CALLBACK_COMPLETE;
    }

    if(value < 0){
        ulfius_set_json_body_response(pResponse, 500, json_pack("{ss}", ERROR, DOOR_ERR));
        return U_CALLBACK_COMPLETE;
    }

    ulfius_set_json_body_response(pResponse, 200, json_pack("{si}", VALUE, value));

    return U_CALLBACK_COMPLETE;

}

/**
 * Turn on/off a light using the gpio library, according
 * to the state specified in the URL
 *
 * @param  pRequest  Http request received
 * @param  pResponse Response object
 * @param  pUserData User Data object
 * @return           json_t object
 */
int callback_lights(const struct _u_request *pRequest, struct _u_response *pResponse, void *pUserData){

    int result = -1;
    char *location = u_map_get(pRequest->map_url, "location");
    char *state = u_map_get(pRequest->map_url, "state");
    int stateValue = atoi(state);

    fprintf(_logFD, "---> LIGHT REQUEST: room -> %s, state -> %d\n", location, stateValue);
    fflush(_logFD);

    if(strcmp(location, "kitchen") == 0)
        result = digitalWrite(LIGHTS[KITCHEN], stateValue);
    else if(strcmp(location, "room1") == 0)
        result = digitalWrite(LIGHTS[BEDROOM_1], stateValue);
    else if(strcmp(location, "room2") == 0)
        result = digitalWrite(LIGHTS[BEDROOM_2], stateValue);
    else if(strcmp(location, "living") == 0)
        result = digitalWrite(LIGHTS[LIVING], stateValue);
    else if(strcmp(location, "dining") == 0)
        result = digitalWrite(LIGHTS[DINING], stateValue);
    else if(strcmp(location, "bath") == 0)
        result = digitalWrite(LIGHTS[BATHROOM], stateValue);
    else{
        ulfius_set_json_body_response(pResponse, 400, json_pack("{ss}", ERROR, ROOM_ERR));
        fprintf(_logFD, "---> LIGHT ERROR: %s\n", ROOM_ERR);
        return U_CALLBACK_COMPLETE;
    }

    if(result < 0){
        ulfius_set_json_body_response(pResponse, 500, json_pack("{ss}", ERROR, LIGHT_ERR));
        fprintf(_logFD, "---> LIGHT ERROR: %s\n", LIGHT_ERR);
        return U_CALLBACK_COMPLETE;
    }

    ulfius_set_json_body_response(pResponse, 200, json_pack("{ss}", OK, LIGHT_OK));

    fprintf(_logFD, "---> LIGHT TURNED ON/OFF!\n");
    fflush(_logFD);
    return U_CALLBACK_CONTINUE;
}

/**
 * Returns the state of all doors
 *
 * @param  pRequest  Http request received
 * @param  pResponse Response object
 * @param  pUserData User Data object
 * @return           json_t object
 */
int callback_all_doors(const struct _u_request *pRequest, struct _u_response *pResponse, void *pUserData){

    fprintf(_logFD, "---> DOORS REQUEST\n");
    fflush(_logFD);
    json_t *root = json_object();

    json_object_set_new(root, "room1", json_integer(digitalRead(DOORS[BEDROOM_1_DOOR])));
    json_object_set_new(root, "room2", json_integer(digitalRead(DOORS[BEDROOM_2_DOOR])));
    json_object_set_new(root, "bathroom", json_integer(digitalRead(DOORS[BATHROOM_DOOR])));
    json_object_set_new(root, "main", json_integer(digitalRead(DOORS[MAIN_DOOR])));
    json_object_set_new(root, "garage", json_integer(digitalRead(DOORS[GARAGE_DOOR])));

    ulfius_set_json_body_response(pResponse, 200, root);
    return U_CALLBACK_COMPLETE;

}

/**
 *  Return a random image encoded in a base64 string
 *
 * @param  pRequest  Http request received
 * @param  pResponse Response object
 * @param  pUserData User Data object
 * @return           json_t object
 */
int callback_picture(const struct _u_request *pRequest, struct _u_response *pResponse, void *pUserData){

    fprintf(_logFD, "---> PICTURE REQUEST\n");
    fflush(_logFD);
    char *path = NULL;

    int optionNum = (rand()%3) + 1;

    switch(optionNum){
        case 1:
            path = PICTURE_1;
            break;
        case 2:
            path = PICTURE_2;
            break;
        case 3:
            path = PICTURE_3;
            break;
        default:
            path = PICTURE_1;
            break;
    }

    //Read image to buffer
    struct ImageData image = readImage(path);
    if(image.status == 0){
        char *base64Encoded = base64(image.buffer, image.size); //Encode image to base64 string
        ulfius_set_json_body_response(pResponse, 200, json_pack("{ss}", "image", base64Encoded));
        fprintf(_logFD, "---> SENDING PICTURE -> %s\n", path);
    }

    else{
        ulfius_set_json_body_response(pResponse, 200, json_pack("{ss}", ERROR, PIC_ERR));
        fprintf(_logFD, "---> PICTURE ERROR: %s\n", PIC_ERR);
    }

    fflush(_logFD);
    return U_CALLBACK_COMPLETE;
}

/**
 * Callback function for the web application on /helloworld url call
 */
int callback_hello_world (const struct _u_request * request, struct _u_response * response, void * user_data) {
  ulfius_set_string_body_response(response, 200, "Hello World!");
  return U_CALLBACK_CONTINUE;
}

/**
 * Add the endpoints that will manage the calls to each URLs
 */
void addEndpoints(struct _u_instance *pServerInstance){

    ulfius_add_endpoint_by_val(pServerInstance, "GET", "/helloworld", NULL, 0, &callback_hello_world, NULL);
    ulfius_add_endpoint_by_val(pServerInstance, "GET", "/home/doors", NULL, 0, &callback_all_doors, NULL);
    ulfius_add_endpoint_by_val(pServerInstance, "GET", NULL, "/users/:user", 0, &callback_login, NULL);
    ulfius_add_endpoint_by_val(pServerInstance, "GET", NULL, "/home/:location/door", 0, &callback_doors, NULL);
    ulfius_add_endpoint_by_val(pServerInstance, "GET", NULL, "/home/:location/light/:state", 0, &callback_lights, NULL);
    ulfius_add_endpoint_by_val(pServerInstance, "GET", "/home/picture", NULL, 0, &callback_picture, NULL);
}

/**
 * Uses the gpio library to export the gpio pins that will be used
 */
void exportPins(){
    fprintf(_logFD, "Exporting pins...\n");
    fflush(_logFD);

    //Export output pins
    int lenght = sizeof(LIGHTS) / sizeof(LIGHTS[0]);
    for(int i=0; i< lenght; i++){
        if(pinMode(LIGHTS[i], OUTPUT) < 0) {
            fprintf(_logFD, "Unable to set pin as ouput: %d\n", LIGHTS[i]);
        }
    }

    //Export input pins
    lenght = sizeof(DOORS) / sizeof(DOORS[0]);
    for(int i=0; i< lenght; i++){
        if(pinMode(DOORS[i], INPUT) < 0) {
            fprintf(_logFD, "Unable to set pin as input: %d\n", DOORS[i]);
        }
    }
}

/**
 * Release the pins exported
 */
void unexportPins(){
    fprintf(_logFD, "Releasing pins...\n");
    fflush(_logFD);
    //Unexport output pins
    int lenght = sizeof(LIGHTS) / sizeof(LIGHTS[0]);
    for(int i=0; i< lenght; i++){
        if(releasePin(LIGHTS[i]) < 0) {
            fprintf(_logFD, "Unable to unexport output pin: %d\n", LIGHTS[i]);
        }
    }

    //Unexport input pins
    lenght = sizeof(DOORS) / sizeof(DOORS[0]);
    for(int i=0; i< lenght; i++){
        if(releasePin(DOORS[i]) < 0) {
            fprintf(_logFD, "Unable to unexport input pin: %d\n", DOORS[i]);
        }
    }
}


/**
 * Resets the contents of the file used for input commands
 * @param pFilePtr File descriptor
 */
void resetInputFile(FILE *pFilePtr){
    pFilePtr = fopen(INPUT_FILE, "w+");
    fclose(pFilePtr);
}

/**
 * Releases the pins used and stop the server framework
 * @param pLogPtr File descriptor of the Log file
 */
void shutdownServer(){
    unexportPins();
    ulfius_stop_framework(&_serverInstance);
    ulfius_clean_instance(&_serverInstance);
    fprintf(_logFD, "Shutting down...\n");
    fclose(_logFD);
}

/**
 * main function
 */
int main(void) {

    srand(time(NULL));
    // ------------------------------------- Daemon
    pid_t process_id = 0;
    pid_t sid = 0;
    // Create child process
    process_id = fork();
    // Indication of fork() failure
    if (process_id < 0) {
        printf("fork failed!\n");
        // Return failure in exit status
        exit(1);
    }
    // PARENT PROCESS. Need to kill it.
    if (process_id > 0) {
        printf("process_id of child process %d \n", process_id);
        // return success in exit status
        exit(0);
    }

    umask(0); //unmask the file mode
    sid = setsid();//set new session
    if(sid < 0) {
        exit(1); // Return failure
    }
    // Change the current working directory to root.
    chdir("/");

    // Close stdin. stdout and stderr
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    //Open a log file
    _logFD = fopen (LOG_FILE, "w+");
    // ------------------------------------- Daemon

    //_publicKeyBuffer = loadKey("/home/root/public.pem");
    _privateKeyBuffer = loadKey(PRIVATE_KEY_FILE);

    // fprintf(_logFD, "KEY: %s\n", _publicKeyBuffer);
    // fflush(_logFD);

    // encrypted_length= public_encrypt(PASS, strlen(PASS), _publicKeyBuffer, encrypted);
    // if(encrypted_length == -1)
    // {
    //     fprintf(_logFD, "Private Encrypt failed\n");
    //     fflush(_logFD);
    // }
    //
    //
    // unsigned char  decrypted[4098]={};
    //
    // private_decrypt(encrypted, encrypted_length, _privateKeyBuffer, decrypted);
    // fprintf(_logFD, "DECRYPTED: %s\n", decrypted);
    // fflush(_logFD);
    //
    // char * base64Encrypt = base64(encrypted, encrypted_length);
    // fprintf(_logFD, "BASE64 ENCRYPTED: %s\n", base64Encrypt);
    // fflush(_logFD);


    //-------------


    char cmd[10];
    int len = 0;

    exportPins();

    // Initialize instance with the port number
    if (ulfius_init_instance(&_serverInstance, PORT, NULL, NULL) != U_OK) {
        fprintf(_logFD, "Error initializing the server instance\n");
        fflush(_logFD);
        return(1);
    }

    addEndpoints(&_serverInstance);

    // Start the framework
    int start = ulfius_start_framework(&_serverInstance) ;
    if (start == U_OK) {
        FILE *inputPtr;
        FILE *tmpPtr;
        char out[100];
        char in[100];

        fprintf(_logFD, "Framework started on port %d\n", _serverInstance.port);
        fflush(_logFD);
        resetInputFile(tmpPtr);

        while(!_EXIT){

            sleep(1);
            if((inputPtr = fopen(INPUT_FILE, "r")) == NULL){
                //printf("Error opening input file\n");
                fprintf(_logFD, "Error opening input file... Shutting down...\n");
                fflush(_logFD);
                _EXIT = 1;
            }
            else{
                fscanf(inputPtr, "%[^\n]", in);
                if(strcmp(in, "exit") == 0){
                    fclose(inputPtr);
                    resetInputFile(tmpPtr);
                    shutdownServer();
                    _EXIT = 1;
                }

                fclose(inputPtr);
                resetInputFile(tmpPtr);
            }
        }

        //     fgets(cmd, 10, stdin);
        //     len = strlen(cmd)-1;
        //     if( cmd[len] == '\n')
        //     cmd[len] = '\0';
        //     if(strcmp(cmd, "exit") == 0)
        //         _EXIT = 1;
        // }
    }

    else {
        fprintf(_logFD, "Error starting framework\n");
        fflush(_logFD);
        shutdownServer();
    }

    fclose(_logFD);
    return 0;
}
