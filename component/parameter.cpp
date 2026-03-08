#include "parameter.h"

Vision::Vision()
{
	;
}

Vision::~Vision()
{
	;
}

// void queue_::forwardInsert(Vision num)
// {

// 	if (length == 0)
// 	{
// 		element_ *ptr = new element_;
// 		ptr->num = num;
// 		ptr->front_p = first_p;
// 		first_p->next_p = ptr;
// 		now_p = ptr;
// 		length++;
// 	}
// 	else
// 	{
// 		element_ *ptr = new element_;
// 		ptr->num = num;
// 		ptr->front_p = first_p;
// 		ptr->next_p = first_p->next_p;
// 		first_p->next_p->front_p = ptr;
// 		first_p->next_p = ptr;
// 		length++;
// 	}
// }

// void queue_::rearwardInsert(Vision num)
// {

// 	if (length == 0)
// 	{
// 		element_ *ptr = new element_;
// 		ptr->num = num;
// 		ptr->front_p = first_p;
// 		first_p->next_p = ptr;
// 		now_p = ptr;
// 		length++;
// 	}
// 	else
// 	{
// 		element_ *ptr = new element_;
// 		ptr->num = num;
// 		ptr->front_p = now_p;
// 		now_p->next_p = ptr;
// 		now_p = ptr;
// 		length++;
// 	}
// }

// bool queue_::getFirstElement(Vision *num)
// {
// 	if (length)
// 	{
// 		*num = first_p->next_p->num;
// 		return true;
// 	}
// 	return false;
// }

// bool queue_::getLastElement(Vision *num)
// {
// 	if (length)
// 	{
// 		*num = now_p->num;
// 		return true;
// 	}
// 	return false;
// }

// int queue_::getLength(void)
// {
// 	return length;
// }

// bool queue_::ifEmpty(void)
// {
// 	return !length;
// }

// bool queue_::forwardPop(Vision *num)
// {
// 	if (length > 1)
// 	{
// 		*num = first_p->next_p->num;
// 		first_p->next_p = first_p->next_p->next_p;
// 		delete first_p->next_p->front_p;
// 		length--;
// 		return true;
// 	}
// 	else if (length == 1)
// 	{
// 		*num = first_p->next_p->num;
// 		delete first_p->next_p;
// 		length--;
// 		return true;
// 	}
// 	else if (length == 0)
// 	{
// 		return false;
// 	}
// 	else
// 	{
// 		return false;
// 	}
// }

// bool queue_::rearwardPop(Vision *num)
// {
// 	if (length > 1)
// 	{
// 		*num = now_p->num;
// 		now_p = now_p->front_p;
// 		delete now_p->next_p;
// 		length--;
// 		return true;
// 	}
// 	else if (length == 1)
// 	{
// 		*num = now_p->num;
// 		delete now_p;
// 		length--;
// 		return true;
// 	}
// 	else if (length == 0)
// 	{
// 		return false;
// 	}
// 	else
// 	{
// 		return false;
// 	}
// }

// queue_::~queue_(void)
// {
// 	Vision *p = new Vision;

// 	while (length)
// 	{
// 		queue_::rearwardPop(p);
// 	}

// 	delete p;
// 	delete first_p;
// }

std::queue<Vision> Vision_queue;
Vision pVision;
Vision VisionTmp;
struct Robot_STATUS robot_status;
struct Robot_CMD robot_cmd;

std::mutex _robot_cmd_mutex;
std::mutex _robot_status_mutex;
std::mutex _pVision_mutex;
std::mutex _vision_queue_mutex;
