#ifndef PRIORITY_QUEUE_
#define PRIORITY_QUEUE_

#define MAXNUM 100
template <class T> class PriorityQueue
{
public:
	T Queue[MAXNUM];
	float Priority[MAXNUM];



	PriorityQueue()
	{
		nums = 0;
	}
	~PriorityQueue()
	{
		nums = 0;
	}


	bool isEmpty()
	{
		if (nums == 0)
			return true;
		else
			return false;
	}

	bool EnterQueue(T t, int priority)
	{
		int counter = nums - 1;
		if (nums >= MAXNUM)
			return false;
		else
		{
			while (counter > 0 && priority < Priority[counter])
			{
				Queue[counter + 1] = Queue[counter];
				Priority[counter + 1] = Priority[counter];
				counter--;
			}
			Queue[counter + 1] = t;
			Priority[counter + 1] = priority;
			nums++;
			return true;
		}
	}

	T DeQueue()
	{
		if (nums > 0)
		{
			int counter = 0;
			T head = Queue[0];
			while (counter < nums - 1)
			{
				Queue[counter] = Queue[counter + 1];
				Priority[counter] = Priority[counter + 1];
				counter++;
			}
			nums--;
			return head;
		}
		else
			return 0;
	}
private:
	int nums;

};



#endif //  _PRIORITY_QUEUE_
