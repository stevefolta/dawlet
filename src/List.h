#ifndef List_h
#define List_h

template<class T>
class List {
	public:
		List() : head(nullptr), last(nullptr) {}

		class iterator {
			public:
				iterator(T* elementIn) : element(elementIn) {}

				void	operator++() {
					if (element)
						element = element->next;
					}
				T*& operator*() { return element; }
				bool	operator==(iterator other) { return element == other.element; }
				bool	operator!=(iterator other) { return element != other.element; }

			protected:
				T*	element;
			};

		iterator	begin() { return iterator(head); }
		iterator end() { return iterator(nullptr); }

		bool	empty() { return head == nullptr; }
		T*	front() { return head; }
		T* back() { return last; }

		void	push_back(T* element) {
			if (last)
				last->next = element;
			else
				head = element;
			last = element;
			}
		void	push_front(T* element) {
			element->next = head;
			head = element;
			if (last == nullptr)
				last = element;
			}
		T*	pop_front() {
			T* element = head;
			if (head) {
				head = head->next;
				if (last == element)
					last = nullptr;
				}
			return element;
			}

		void	remove(T* removing_element) {
			if (removing_element == head) {
				pop_front();
				return;
				}

			// Find where to remove it.
			for (T* element = head; element; element = element->next) {
				if (element->next == removing_element) {
					// Found the spot.  Remove it.
					element->next = removing_element->next;
					if (last == removing_element)
						last = element;
					break;
					}
				}
			}

	protected:
		T*	head;
		T*	last;
	};


#endif 	// List_h

