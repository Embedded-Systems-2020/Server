#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gpio.h>
#include <ulfius.h>
#include <consts.h>
#include <rsa.h>
#include <search.h>

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
            fprintf(stderr, "Unable to set pin as ouput: %d", LIGHTS[i]);
        }
    }

    //Export input pins
    lenght = sizeof(DOORS) / sizeof(DOORS[0]);
    for(int i=0; i< lenght; i++){
        if(pinMode(DOORS[i], INPUT) < 0) {
            fprintf(stderr, "Unable to set pin as input: %d", DOORS[i]);
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
            fprintf(stderr, "Unable to unexport pin: %d", LIGHTS[i]);
        }
    }

    //Unexport input pins
    lenght = sizeof(DOORS) / sizeof(DOORS[0]);
    for(int i=0; i< lenght; i++){
        if(releasePin(DOORS[i]) < 0) {
            fprintf(stderr, "Unable to unexport pin: %d", DOORS[i]);
        }
    }
}

/**
 * main function
 */
int main(void) {

    char cmd[10];
    int len = 0;

    exportPins();

    // Initialize instance with the port number
    if (ulfius_init_instance(&_serverInstance, PORT, NULL, NULL) != U_OK) {
        fprintf(stderr, "Error initializing the server instance\n");
        return(1);
    }

    addEndpoints(&_serverInstance);

    // Start the framework
    int start = ulfius_start_framework(&_serverInstance) ;
    if (start == U_OK) {
        printf("Start framework on port %d\n", _serverInstance.port);

        //Exit from terminal
        while(!_EXIT){
            fgets(cmd, 10, stdin);
            len = strlen(cmd)-1;
            if( cmd[len] == '\n')
            cmd[len] = '\0';
            if(strcmp(cmd, "exit") == 0)
                _EXIT = 1;
        }
    }

    else {
        fprintf(stderr, "Error starting framework\n");
        printf("Error is: %d\n", start);
    }

    printf("End framework\n");

    //Release pins and stop server framework
    unexportPins();
    ulfius_stop_framework(&_serverInstance);
    ulfius_clean_instance(&_serverInstance);

    return 0;
}
