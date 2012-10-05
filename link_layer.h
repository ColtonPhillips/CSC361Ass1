#include <pthread.h>
#include <unistd.h>
#include <exception>

#include "physical_layer.h"

// TODO: send_queue\data:
/*
	send_queue: array of timed_packet
		must be implemented as an array of timed_packet
		packets must be stored in arrival order,
		with the oldest packet at the left and
		the most recent packet at the right
*/

class Link_layer_exception: public exception
{};

struct Packet_header {
	unsigned int checksum;
	unsigned int seq;
	unsigned int ack;
	unsigned int data_length;
};

struct Packet {
	struct Packet_header header;
	unsigned char data[Physical_layer_interface::MAXIMUM_BUFFER_LENGTH-
	 sizeof(struct Packet_header)];
};

struct Timed_packet {
	timeval send_time;
	struct Packet packet;
};

class Link_layer {
public:
	enum {MAXIMUM_DATA_LENGTH = 
	 Physical_layer_interface::MAXIMUM_BUFFER_LENGTH-sizeof(Packet_header)};

	Link_layer(Physical_layer_interface* physical_layer_interface,
	 unsigned int num_sequence_numbers,
	 unsigned int max_send_window_size,unsigned int timeout);

	unsigned int send(unsigned char buffer[], unsigned int length);

	unsigned int receive(unsigned char buffer[]);
private:
	Physical_layer_interface* physical_layer_interface;
	unsigned int num_sequence_numbers;
	unsigned int max_send_window_size;
	timeval timeout;

	pthread_t thread;

	// to prevent race condition using mutex
	pthread_mutex_t mutex; 

	// seq for next packet added to send_queue
	unsigned int next_send_seq;

	// next send ack
	unsigned int next_send_ack;

	// seq of next packet expected from PL
	unsigned int next_receive_seq;

	// last ack received from PL
	unsigned int last_receive_ack;

	unsigned char receive_buffer[MAXIMUM_DATA_LENGTH];
	unsigned int receive_buffer_length;

	static void* loop(void* link_layer);
	void process_received_packet(struct Packet p);
	void remove_acked_packets();
	void send_timed_out_packets();
	void generate_ack_packet();
};
