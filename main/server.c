#include <stdio.h>
#include <gpio.h>
#include <ulfius.h>
#include <consts.h>

#define PORT 8080
int _EXIT = 0;

int callback_test_light(const struct _u_request *request, struct _u_response *response, void *user_data){

    char *state = u_map_get(request->map_url, "state");
    int stateValue = atoi(state);

    if(digitalWrite(LIGHTS[TEST], stateValue) == 0)
        ulfius_set_json_body_response(response, 200, json_pack("{ss}", OK, LIGHT_OK));
    else
        ulfius_set_json_body_response(response, 500, json_pack("{ss}", ERROR, LIGHT_ERR));

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
 * main function
 */
int main(void) {

//     FILE *cmdFP;
//     char *line = NULL;
//     int len = 32;
    char cmd[10];
    int len = 0;

    //Initialize pin
    if(pinMode(LIGHTS[TEST], OUTPUT) < 0) {
        fprintf(stderr, "Unable to set pin Mode\n");
        return(1);
    }

    struct _u_instance instance;

    // Initialize instance with the port number
    if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
        fprintf(stderr, "Error ulfius_init_instance, abort\n");
        return(1);
    }

    // Endpoint list declaration
    ulfius_add_endpoint_by_val(&instance, "GET", "/helloworld", NULL, 0, &callback_hello_world, NULL);

    ulfius_add_endpoint_by_val(&instance, "GET", NULL, "/led/:state", 0, &callback_test_light, NULL);

    // Start the framework
    if (ulfius_start_framework(&instance) == U_OK) {
        printf("Start framework on port %d\n", instance.port);

        // while(1){
        //     cmdFP = fopen("/home/root/cmd", "r");
        //     if(cmdFP == NULL){
        //         cmdFP = fopen("/home/root/cmd", "w+");
        //     }
        //     else{
        //         if(fgets(line, len, cmdFP) != NULL){
        //             if(strcmp(line, "kill")){
        //                 fclose(cmdFP);
        //                 remove("/home/root/cmd");
        //                 cmdFP = fopen("/home/root/cmd", "w+");
        //                 fclose(cmdFP);
        //                 printf("Killing the program\n");
        //                 break;
        //             }
        //             else{
        //                 line = NULL;
        //                 fclose(cmdFP);
        //             }
        //         }
        //         else{
        //             fclose(cmdFP);
        //         }
        //     }
        // }

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
    }

    printf("End framework\n");

    releasePin(LIGHTS[TEST]);

    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);

    return 0;
}
