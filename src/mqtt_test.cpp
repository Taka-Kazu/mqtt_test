#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mutex>
#include <mosquitto.h>

char *sub_topic   = "topic0";
char *pub_topic   = "topic0";
char *message = "message";
volatile bool disconnected = false;
std::mutex mtx;

void on_connect(struct mosquitto *mosq, void *obj, int result)
{
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
    mosquitto_subscribe(mosq, NULL, sub_topic, 0);
}

void on_disconnect(struct mosquitto *mosq, void *obj, int rc)
{
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
    mtx.lock();
    disconnected = true;
    mtx.unlock();
    printf("\033[31mdisconnected from broker!!!\n\033[0m");
    mosquitto_loop_stop(mosq, true);
}

void on_publish(struct mosquitto *mosq, void *userdata, int mid)
{
    printf("on_publish\n");
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    if(message->payloadlen){
        printf("%s ", message->topic);
        fwrite(message->payload, 1, message->payloadlen, stdout);
        printf("%lf", *((double *)message->payload));
        printf("\n");
    }else{
        printf("%s (null)\n", message->topic);
    }
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    char *id            = "mqtt_test";
    char *host          = "localhost";
    int   port          = 1883;
    int   keepalive     = 60;
    bool  clean_session = true;
    struct mosquitto *mosq = NULL;

    mosquitto_lib_init();
    mosq = mosquitto_new(id, clean_session, NULL);
    if(!mosq){
        fprintf(stderr, "Cannot create mosquitto object\n");
        mosquitto_lib_cleanup();
        return(EXIT_FAILURE);
    }
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_disconnect_callback_set(mosq, on_disconnect);
    mosquitto_publish_callback_set(mosq, on_publish);
    mosquitto_message_callback_set(mosq, on_message);

    if(mosquitto_connect_bind(mosq, host, port, keepalive, NULL)){
        fprintf(stderr, "failed to connect broker.\n");
        mosquitto_lib_cleanup();
        return(EXIT_FAILURE);
    }

    mosquitto_loop_start(mosq);

    while(1){
        mtx.lock();
        if(disconnected){
            break;
        }
        mtx.unlock();
        printf("loop\n");
        double data = 3.141592;
        // mosquitto_publish(mosq, NULL, pub_topic, strlen(message), message, 0, 0);
        mosquitto_publish(mosq, NULL, pub_topic, sizeof(data), &data, 0, 0);
        sleep(1);
    }

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return(EXIT_SUCCESS);
}

