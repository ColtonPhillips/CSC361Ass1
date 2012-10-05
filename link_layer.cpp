#include "link_layer.h"
#include "timeval_operators.h"
unsigned short checksum(struct Packet);

Link_layer::Link_layer(Physical_layer_interface* physical_layer_interface,
 unsigned int num_sequence_numbers,
 unsigned int max_send_window_size,unsigned int timeout)
{
	next_send_seq = 0;
	next_send_ack = 0;
	next_receive_seq = 0;
	last_receive_ack = 0;

	this->physical_layer_interface = physical_layer_interface;

	receive_buffer_length = 0;

	pthread_mutex_init(&mutex, NULL);

	if (pthread_create(&thread,NULL,&Link_layer::loop,this) < 0) {
		throw Link_layer_exception();
	}
}

unsigned int Link_layer::send(unsigned char buffer[],unsigned int length)
{
	/*	
	if data length is 0 or exceeds maximum
		throw link_layer_exception
	if send_queue not full
		// add a new timed_packet P
		set P.send_time to now
		copy buffer to P.packet.data
		set P.packet.header.data_length to length
		set P.packet.header.seq to next_send_seq
		add P to right end of send_queue

		increment next_send_seq

		return true
	else 
		return false
	*/

	unsigned int n = physical_layer_interface->send(buffer,length);

	return n;
}

unsigned int Link_layer::receive(unsigned char buffer[])
{
	/*
	if receive_buffer contains data of length N:
		copy receive_buffer into buffer
		empty receive_buffer
		return N
	else
		return 0
	*/

	unsigned int length;
	pthread_mutex_lock(&mutex);

	length = receive_buffer_length;
	if (length > 0) {
		for (unsigned int i = 0; i < length; i++) {
			buffer[i] = receive_buffer[i];
		}
		receive_buffer_length = 0;
	}
	pthread_mutex_unlock(&mutex);
	return length;
}

void Link_layer::process_received_packet(struct Packet p)
{
	/*if p.seq == next_receive_seq
		if p.data_length > 0
			if receive_buffer empty
				copy packet data to receive_buffer
				increment next_receive_seq
		else
			increment next_receive_seq
	last_receive_ack = p.ack*/
}

void Link_layer::remove_acked_packets()
{
	/*if send_queue contains a packet acknowldged by last_receive_ack
		remove that packet and all packets to its left*/
}

void Link_layer::send_timed_out_packets()
{
	/*if send_queue contains a packet acknowldged by last_receive_ack
		remove that packet and all packets to its left*/
}

void Link_layer::generate_ack_packet()
{
	/*if send_queue is empty
		add a new timed_packet P to send_queue
		P.send_time = now
		P.seq = next_send_seq
		P.ack = don't care
		P.data_length = 0

		increment next_send_seq*/
}

void* Link_layer::loop(void* thread_creator)
{
	const unsigned int LOOP_INTERVAL = 10;
	Link_layer* link_layer = ((Link_layer*) thread_creator);

	while (true) {

		pthread_mutex_lock(&link_layer->mutex);

		if (link_layer->receive_buffer_length == 0) {
			unsigned int length =
			 link_layer->physical_layer_interface->receive
			 (link_layer->receive_buffer);

			if (length > 0) {
				link_layer->receive_buffer_length = length;
			}
		}

		pthread_mutex_unlock(&link_layer->mutex);
		
		usleep(LOOP_INTERVAL);
	}

	return NULL;
}

// this is the standard Internet checksum algorithm
unsigned short checksum(struct Packet p)
{
	unsigned long sum = 0;
	struct Packet copy;
	unsigned short* shortbuf;
	unsigned int length;

	if (p.header.data_length > Link_layer::MAXIMUM_DATA_LENGTH) {
		throw Link_layer_exception();
	}

	copy = p;
	copy.header.checksum = 0;
	length = sizeof(Packet_header)+copy.header.data_length;
	shortbuf = (unsigned short*) &copy;

	while (length > 1) {
		sum += *shortbuf++;
		length -= 2;
	}
	// handle the trailing byte, if present
	if (length == 1) {
		sum += *(unsigned char*) shortbuf;
	}

	sum = (sum >> 16)+(sum & 0xffff);
	sum = (~(sum+(sum >> 16)) & 0xffff);
	return (unsigned short) sum;
}
