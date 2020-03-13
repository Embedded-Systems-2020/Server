/**
 * test.c
 * Small Hello World! example
 * to compile with gcc, run the following command
 * gcc -o test test.c -lulfius
 */
#include <stdio.h>
#include <gpio.h>
#include <ulfius.h>
#include <jansson.h>

#define PORT 8080
#define PIN_TEST 24

int callback_test_led(const struct _u_request *request, struct _u_response *response, void *user_data){
    char *state = u_map_get(request->map_url, "state");
    int stateValue = atoi(state);

    if(digitalWrite(PIN_TEST, stateValue) == 0)
        ulfius_set_json_body_response(response, 200, json_pack("{ss}", "message", "Led state changed!"));
    else
        ulfius_set_string_body_response(response, 500, "Error changing led state");

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

    //Initialize pin
    if(pinMode(PIN_TEST, OUTPUT) < 0) {
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

    ulfius_add_endpoint_by_val(&instance, "GET", NULL, "/led/:state", 0, &callback_test_led, NULL);

    // Start the framework
    if (ulfius_start_framework(&instance) == U_OK) {
        printf("Start framework on port %d\n", instance.port);

        while(1){

//             cmdFP = fopen("/home/root/cmd", "r");
//             if(cmdFP == NULL){
//                 cmdFP = fopen("/home/root/cmd", "w+");
//             }
//             else{
//                 if(fgets(line, len, cmdFP) != NULL){
//                     if(strcmp(line, "kill")){
//                         fclose(cmdFP);
//                         remove("/home/root/cmd");
//                         cmdFP = fopen("/home/root/cmd", "w+");
//                         fclose(cmdFP);
//                         printf("Killing the program\n");
//                         break;
//                     }
//                     else{
//                         line = NULL;
//                         fclose(cmdFP);
//                     }
//                 }
//                 else{
//                     fclose(cmdFP);
//                 }
//
//             }
        }

        // Wait for the user to press <enter> on the console to quit the application
        //getchar();
    }

    else {
        fprintf(stderr, "Error starting framework\n");
    }

    printf("End framework\n");

    releasePin(PIN_TEST);

    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);

    return 0;
}
