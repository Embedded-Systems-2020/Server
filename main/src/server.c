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

#define PORT 8080
static const char *USER = "admin";
static const char *PASS = "embedded2020";
    struct _u_instance _serverInstance;
int _EXIT = 0;

int callback_login(const struct _u_request *pRequest, struct _u_response *pResponse, void *pUserData){

    char *user = u_map_get(pRequest->map_url, "user");
    char *pass = u_map_get(pRequest->map_header, "pass");

    //Checks if both user and pass have been provided
    if(user == NULL || pass == NULL) {
        ulfius_set_json_body_response(pResponse, 400, json_pack("{ss}", ERROR, LOGIN_ERR));
        return U_CALLBACK_COMPLETE;
    }

    if(strcmp(user, USER) == 0){
        if(strcmp(pass, PASS) == 0)
            ulfius_set_json_body_response(pResponse, 200, json_pack("{ss}", OK, LOGIN_OK));
        else
            ulfius_set_json_body_response(pResponse, 401, json_pack("{ss}", ERROR, PASS_ERR));
    }
    else
        ulfius_set_json_body_response(pResponse, 404, json_pack("{ss}", ERROR, USER_ERR));

    return U_CALLBACK_COMPLETE;
}

int callback_doors(const struct _u_request *pRequest, struct _u_response *pResponse, void *pUserData){

    int value = -1;
    char *location = u_map_get(pRequest->map_url, "location");
    if(location == NULL){
        ulfius_set_json_body_response(pResponse, 400, NULL);
        return U_CALLBACK_COMPLETE;
    }

    if(strcmp(location, "kitchen") == 0)
        value = digitalRead(DOORS[KITCHEN]);
    else if(strcmp(location, "room1") == 0)
        value = digitalRead(DOORS[BEDROOM_1]);
    else if(strcmp(location, "room2") == 0)
        value = digitalRead(DOORS[BEDROOM_2]);
    else if(strcmp(location, "living") == 0)
        value = digitalRead(DOORS[LIVING]);
    else if(strcmp(location, "dining") == 0)
        value = digitalRead(DOORS[DINING]);
    else if(strcmp(location, "bath") == 0)
        value = digitalRead(DOORS[BATHROOM]);
    else if(strcmp(location, "main") == 0)
        value = digitalRead(DOORS[MAIN]);
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


int callback_lights(const struct _u_request *pRequest, struct _u_response *pResponse, void *pUserData){

    int result = -1;
    char *location = u_map_get(pRequest->map_url, "location");
    char *state = u_map_get(pRequest->map_url, "state");
    int stateValue = atoi(state);

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
        return U_CALLBACK_COMPLETE;
    }

    if(result < 0){
        ulfius_set_json_body_response(pResponse, 500, json_pack("{ss}", ERROR, LIGHT_ERR));
        return U_CALLBACK_COMPLETE;
    }

    ulfius_set_json_body_response(pResponse, 200, json_pack("{ss}", OK, LIGHT_OK));

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

    json_t *root = json_object();

    json_object_set_new(root, "kitchen", json_integer(digitalRead(DOORS[KITCHEN])));
    json_object_set_new(root, "room1", json_integer(digitalRead(DOORS[BEDROOM_1])));
    json_object_set_new(root, "room2", json_integer(digitalRead(DOORS[BEDROOM_2])));
    json_object_set_new(root, "living", json_integer(digitalRead(DOORS[LIVING])));
    json_object_set_new(root, "dining", json_integer(digitalRead(DOORS[DINING])));
    json_object_set_new(root, "bathroom", json_integer(digitalRead(DOORS[BATHROOM])));
    json_object_set_new(root, "main", json_integer(digitalRead(DOORS[MAIN])));

    ulfius_set_json_body_response(pResponse, 200, root);
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
}

/**
 * Uses the gpio library to export the gpio pins that will be used
 */
void exportPins(){

    //Export output pins
    int lenght = sizeof(LIGHTS) / sizeof(LIGHTS[0]);
    for(int i=0; i< lenght; i++){
        if(pinMode(LIGHTS[i], OUTPUT) < 0) {
            fprintf(stderr, "Unable to set pin as ouput: %d\n", LIGHTS[i]);
        }
    }

    //Export input pins
    lenght = sizeof(DOORS) / sizeof(DOORS[0]);
    for(int i=0; i< lenght; i++){
        if(pinMode(DOORS[i], INPUT) < 0) {
            fprintf(stderr, "Unable to set pin as input: %d\n", DOORS[i]);
        }
    }
}

/**
 * Release the pins exported
 */
void unexportPins(){
    //Unexport output pins
    int lenght = sizeof(LIGHTS) / sizeof(LIGHTS[0]);
    for(int i=0; i< lenght; i++){
        if(releasePin(LIGHTS[i]) < 0) {
            fprintf(stderr, "Unable to unexport pin: %d\n", LIGHTS[i]);
        }
    }

    //Unexport input pins
    lenght = sizeof(DOORS) / sizeof(DOORS[0]);
    for(int i=0; i< lenght; i++){
        if(releasePin(DOORS[i]) < 0) {
            fprintf(stderr, "Unable to unexport pin: %d\n", DOORS[i]);
        }
    }
}

/**
 * Resets the contents of the file used for input commands
 * @param pFilePtr File descriptor
 */
void resetInputFile(FILE *pFilePtr){
    pFilePtr = fopen("srv_input.txt", "w+");
    fclose(pFilePtr);
}

void shutdownServer(FILE *pLogPtr){
    unexportPins();
    ulfius_stop_framework(&_serverInstance);
    ulfius_clean_instance(&_serverInstance);
    fprintf(pLogPtr, "Shutting down...\n");
    fclose(pLogPtr);
}

/**
 * main function
 */
int main(void) {
    //---------------------------------------------------------------------------------
        FILE *fp= NULL;
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
        //unmask the file mode
        umask(0);
        //set new session
        sid = setsid();
        if(sid < 0) {
            // Return failure
            exit(1);
        }
        // Change the current working directory to root.
        chdir("/");
        // Close stdin. stdout and stderr
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        // Open a log file in write mode.
        fp = fopen ("Log_server.txt", "w+");

    //---------------------------------------------------------------------------------

    char cmd[10];
    int len = 0;

    exportPins();

    // Initialize instance with the port number
    if (ulfius_init_instance(&_serverInstance, PORT, NULL, NULL) != U_OK) {
        fprintf(fp, "Error initializing the server instance\n");
        return(1);
    }

    addEndpoints(&_serverInstance);

    // Start the framework
    int start = ulfius_start_framework(&_serverInstance) ;
    if (start == U_OK) {
        //printf("Start framework on port %d\n", _serverInstance.port);

        FILE *inputPtr;
        FILE *tmpPtr;
        char out[100];
        char in[100];

        fprintf(fp, "Framework started on port %d\n", _serverInstance.port);
        fflush(fp);

        resetInputFile(tmpPtr);

        //Exit from terminal
        while(!_EXIT){

            sleep(1);
            if((inputPtr = fopen("srv_input.txt", "r")) == NULL){
                //printf("Error opening input file\n");
                fprintf(fp, "Error opening input file... Shutting down...\n");
                fflush(fp);
                _EXIT = 1;

            }
            else{
                fscanf(inputPtr, "%[^\n]", in);
                if(strcmp(in, "exit") == 0){
                    fclose(inputPtr);
                    resetInputFile(tmpPtr);
                    shutdownServer(fp);
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
        fprintf(stderr, "Error starting framework\n");
        printf("Error is: %d\n", start);
        fprintf(fp, "Error starting framework\n");
        fflush(fp);

        shutdownServer(fp);
    }


    // //Release pins and stop server framework
    // unexportPins();
    // ulfius_stop_framework(&_serverInstance);
    // ulfius_clean_instance(&_serverInstance);

    fclose(fp);

    return 0;
}
