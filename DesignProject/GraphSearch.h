#pragma once

#include <map>
#include <vector>
#include <list>
#include <queue>
#include <algorithm>

#define ASSERT_VALID_PTR(ptr) do { if ((ptr) == NULL || (ptr == nullptr)) throw std::invalid_argument("invalid pointer"); } while (0)

namespace GraphSearch
{
	#pragma region 

	template<class T, typename U> class Node
	{
	public:
		// constructor and destructor
		Node() // for root nodes
		{
			_object = nullptr;
		}
		Node(T* object)
		{
			ASSERT_VALID_PTR(object);
			_object = object;
		}
		~Node() 
		{
			_children.clear();
		}

		// public methods
		std::map<Node<T, U>*, U>& children() 
		{
			return _children; 
		}
		bool hasChildren() const 
		{ 
			return !_children.empty(); 
		}
		void appendChild(Node<T, U>* const child, U value)
		{
			ASSERT_VALID_PTR(child);

			// check if child already among children
			if (_children.find(child) == _children.end())
			{
				_children[child] = value;
			}
			else
				throw std::invalid_argument("child already in node's children");
		}
		void appendChild(Node<T, U>& child, U value)
		{
			appendChild(&child, value);
		}
		int childrenNumber() const 
		{ 
			return _children.size();
		}
		void clearChildren() 
		{ 
			_children.clear(); 
		}
		std::pair<Node<T, U>*, U> minimumEdge()
		{
			std::map<Node<T, U>*, U>::iterator minimumIt = _children.begin();
			std::map<Node<T, U>*, U>::iterator endIt = _children.end();
			for (std::map<Node<T, U>*, U>::iterator it = std::next(minimumIt, 1); it != endIt; ++it)
				if (it->second < minimumIt->second)
					minimumIt = it;
			return (*minimumIt);
		}
		T* object() const 
		{ 
			return _object; 
		}
		void setObject(T* const object)
		{
			ASSERT_VALID_PTR(object);
			_object = object;
		}
		/*
		NO MORE VALID AS IT IS: void appendChildren(int count, ...) // usage: root.appendChildren(3, &n1, &n2, &n3);
		{
			// intialize arguments list
			va_list args;
			va_start(args, count);
			Node* node;

			// iterate through n arguments and appen node pointers as children
			for (int i = 0; i < count; ++i)
			{
				this->appendChild(va_arg(args, Node*));
			}

			// end arguments list
			va_end(args);
		}
		*/

	protected:
		// protected members
		std::map<Node<T, U>*, U> _children;		// contains children and the cost of moving from this node to each child
		T* _object;								// pointer to object linked with this node
	};

	#pragma endregion Node



	#pragma region 

	template<class T, typename U> class Graph
	{
	public:
		// constructor and destructor
		Graph()
		{
			_root = nullptr;
		}
		Graph(Node<T, U>* const root)
		{
			ASSERT_VALID_PTR(root);
			_root = root;
		}
		Graph(Node<T, U>& root)
		{
			_root = &root;
		}
		~Graph() { }
		
		// Reaches end of graph by choosing at each node the local minimum (the minimal-value child).
		// Starts from startNode (default is Root, i.e., nullptr).
		// Very fast, but does not grant global minimum.
		// Since it does not choose its path according to a target, it makes no sense to define a 
		// target end node for this algorithm. It runs till no unvisited nodes are found.
		std::vector<Node<T, U>*> greedySearch(Node<T, U>* startNode = nullptr)
		{
			// check if specified startNode is null
			Node<T, U>* actual_startNode = startNode == nullptr ? _root : startNode;

			// add starting node to path
			std::vector<Node<T, U>*> path{ actual_startNode };

			// start search
			if (actual_startNode->hasChildren())
			{
				// calculate minimum-value children
				Node<T, U>* minNode = actual_startNode->minimumEdge().first;
				
				// append minimum to vector path
				path.push_back(minNode);

				// start crawling recursively
				greedySearchHelper(path, minNode);
			}
			
			// return result
			return path;
		}

		// Finds all paths from startNode (Root by default) to endNode (end of the graph by default).
		// Returns them as a list of paths vectors. 
		void depthFirstSearch(std::list<std::vector<Node<T, U>*>>& paths, Node<T, U>* startNode = nullptr, Node<T, U>* endNode = nullptr)
		{
			// clear input vector
			if (!paths.empty())
				paths.clear();

			// check if specified startNode is null
			Node<T, U>* actual_startNode = startNode == nullptr ? _root : startNode;
			
			// start search if startNode has children and it's not the endNode
			if (actual_startNode->hasChildren() && actual_startNode != endNode)
			{
				// create path and add starting node; start recursive search
				std::vector<Node<T, U>*> path;
				path.push_back(actual_startNode);
				depthFirstSearchHelper(path, paths, actual_startNode, endNode);
			}
			else
			{
				// append only the start node to the paths and return
				paths.push_back(std::vector<Node<T, U>*>{ actual_startNode });
			}
		}

		// Finds all paths from startNode (Root by default) to endNode (end of the graph by default).
		// Returns them as a list of paths vectors. 
		void breadthFirstSearch(std::list<std::vector<Node<T, U>*>>& paths, Node<T, U>* startNode = nullptr, Node<T, U>* endNode = nullptr, bool stopAtShortestPath = false)
		{
			// clear input vector
			if (!paths.empty())
				paths.clear();

			// check if specified startNode is null
			Node<T, U>* actual_startNode = startNode == nullptr ? _root : startNode;

			// start search if startNode has children and it's not the endNode
			if (actual_startNode->hasChildren() && actual_startNode != endNode)
			{
				// initialize current path and BFS queue
				std::vector<Node<T, U>*> path{ actual_startNode };
				std::queue<std::vector<Node<T, U>*>> Q;
				Q.push(path);

				// start looping
				while (!Q.empty())
				{
					path = Q.front();
					Q.pop();
					Node<T, U>* lastNode = path.back();

					// save this path to paths list and stop looping on this path iff it has reached end OR has no children OR has no unvisited children for this path
					if (lastNode == endNode)
					{
						paths.push_back(path);
						if (stopAtShortestPath)
							return;
					}
					else if (!lastNode->hasChildren() || allChildrenVisited(path, lastNode))
					{
						if (endNode == nullptr)			// save path iff endNode was not specified
							paths.push_back(path);
						if (stopAtShortestPath)
							return;
					}
					// else, push new paths to queue
					else
					{
						// append each unvisited child of lastNode to a path, and push it to queue
						std::map<Node<T, U>*, U>::iterator endIt = lastNode->children().end();
						for (std::map<Node<T, U>*, U>::iterator it = lastNode->children().begin(); it != endIt; ++it)
						{
							if (std::find(path.begin(), path.end(), it->first) == path.end())
							{
								std::vector<Node<T, U>*> newPath(path);
								newPath.push_back(it->first);
								Q.push(newPath);
							}
						}
					}
				}
			}
			else
			{
				// append only the start node to the paths and return
				paths.push_back(std::vector<Node<T, U>*>{ actual_startNode });
			}
		}

		// Finds the total minimum cost path from startNode (Root by default) to endNode (end of the graph by default). 
		// It minimizes the sum of the costs of traversing from start to end.
		// It uses Dijkstra's algorithm.
		void dijkstra()
		{

		}

		// Finds the total minimum cost path from startNode (Root by default) to endNode (end of the graph by default). 
		// It minimizes the sum of the costs of traversing from start to end.
		// It uses DFS algorithm.
		std::vector<Node<T, U>*> findMinimumCostPath(Node<T, U>* startNode = nullptr, Node<T, U>* endNode = nullptr)
		{
			// perform DFS
			std::list<std::vector<Node<T, U>*>> paths;
			depthFirstSearch(paths, startNode, endNode);

			// iterate over all solutions, saving index of minimum one
			std::vector<Node<T, U>*> minPath = paths.back();
			paths.pop_back();
			U minVal = minPath[0]->children().at(minPath[1]);
			const size_t N = minPath.size();
			for (size_t i = 1; i < N - 1; ++i)
				minVal += minPath[i]->children().at(minPath[i + 1]);

			while (!paths.empty())
			{
				std::vector<Node<T, U>*> path = paths.back();
				paths.pop_back();

				U val = path[0]->children().at(path[1]);
				const size_t N = path.size();
				for (size_t i = 1; i < N - 1; ++i)
					val += path[i]->children().at(path[i + 1]);

				// if (val < minVal)
				if (std::less<U>{}(val, minVal))
				{
					minPath = path;
					minVal = val;
				}
			}

			return minPath;
		}

		// Finds the shortest path from startNode (Root by default) to endNode (end of the graph by default). 
		// It uses BFS algorithm.
		std::vector<Node<T, U>*> findShortestPath(Node<T, U>* startNode = nullptr, Node<T, U>* endNode = nullptr)
		{
			// perform BFS
			std::list<std::vector<Node<T, U>*>> paths;
			breadthFirstSearch(paths, startNode, endNode, true);
			return paths.front();
		}
		
	protected:
		// protected members
		Node<T, U>* _root;
		/* THIS WAS THE METHOD BY WHICH TO EVALUATE A NODE
		U(T::* _evalMethod)(Node*);
		U(T::* _evalConstMethod)() const;

		Graph(Node<T, U>* const root, U(T::* method)())
		{
			ASSERT_VALID_PTR(method);
			_root = root;
			_evalMethod = method;
			_evalConstMethod = nullptr;
		}

		void specifyEvaluationMethod(U(T::* method)() const)
		{
			ASSERT_VALID_PTR(method);
			_evalMethod = nullptr;
			_evalConstMethod = method;
		}
		U evaluateNode(Node<T, U>* from, Node<T, U>* to, U(T::* method)() const)
		{
			ASSERT_VALID_PTR(method);
			return ((node->object()->*method)());
		}

		*/
		
		// protected methods
		bool allChildrenVisited(std::vector<Node<T, U>*>& path, Node<T, U>* node)
		{
			// given a node and a path, returns true if all children are in said path
			std::map<Node<T, U>*, U>::iterator endIt = node->children().end();
			for (std::map<Node<T, U>*, U>::iterator it = node->children().begin(); it != endIt; ++it)
			{
				if (std::find(path.begin(), path.end(), it->first) == path.end())
					return false;
			}
			return true;
		}
		/* DEPRECATED!! std::map<Node<T, U>*, std::vector<Node<T, U>*>> _edges;  void edgesCrawler(Node<T, U>* node)
		{
			/// Populates recursively the map that contains, for each node (vertex), its children connections (edges)

			// check if node has been already traversed
			if (_edges.find(node) == _edges.end())
			{
				// add this node as map key and its children as map value
				_edges[node] = node->children();

				// crawl through all children of this node
				const int N = node->childrenNumber();
				for (int i = 0; i < N; ++i)
					edgesCrawler(node->children()[i]);
			}
		} */

		// private search methods (complementay to public ones)
		void greedySearchHelper(std::vector<Node<T, U>*>& path, Node<T, U>* node)
		{
			// if node has children, find the minimum value one and recursively repeat the function
			// else, the base case is no child,  so the algorithm has reached a deadend.
			if (node->hasChildren())
			{
				// calculate minimum-value children
				Node<T, U>* minNode = node->minimumEdge().first;

				// check if minimum-value node is already in the path. If so, break recursion
				if (std::find(path.begin(), path.end(), minNode) == path.end())
				{
					// set minimum as visited and append to vector
					path.push_back(minNode);

					// start crawling recursively
					greedySearchHelper(path, minNode);
				}
			}
		}
		void depthFirstSearchHelper(std::vector<Node<T, U>*>& path, std::list<std::vector<Node<T, U>*>>& paths, Node<T, U>* node, Node<T, U>* endNode)
		{
			// iterate over all children
			std::map<Node<T, U>*, U>::iterator endIt = node->children().end();
			for (std::map<Node<T, U>*, U>::iterator it = node->children().begin(); it != endIt; ++it)
			{
				// check if current node is target end node
				if (it->first == endNode)
				{
					// stop recursion; save path
					std::vector<Node<T, U>*> tmpPath = path;
					tmpPath.push_back(it->first);
					paths.push_back(tmpPath);
				}
				// check if current node has no children OR EQUIVALENTLY all of its children have been already visited
				else if (!(it->first->hasChildren()) || allChildrenVisited(path, it->first))
				{
					// stop recursion; save path iff endNode was not specified
					if (endNode == nullptr)
					{
						// save current path to paths
						std::vector<Node<T, U>*> tmpPath = path;
						tmpPath.push_back(it->first);
						paths.push_back(tmpPath);
					}
				}
				// check if current node is not in current path
				else if (std::find(path.begin(), path.end(), it->first) == path.end())
				{
					path.push_back(it->first);
					depthFirstSearchHelper(path, paths, it->first, endNode);
					path.pop_back();
				}
			}

			/*
			const int N = node->childrenNumber();
			for (int i = 0; i < N; ++i)
			{
				// check if current node is target end node
				if ((node->children()[i] == endNode))
				{
					// stop recursion; save path
					std::vector<Node<T, U>*> tmpPath = path;
					tmpPath.push_back(node->children()[i]);
					paths.push_back(tmpPath);
				}
				// check if current node has no children
				else if (!(node->children()[i]->hasChildren()))
				{
					// stop recursion; save path iff endNode was not specified
					if (endNode == nullptr)
					{
						// save current path to paths
						std::vector<Node<T, U>*> tmpPath = path;
						tmpPath.push_back(node->children()[i]);
						paths.push_back(tmpPath);
					}
				}
				// check if current node is not in the current path
				else if(std::find(path.begin(), path.end(), node->children()[i]) == path.end())
				{
					path.push_back(node->children()[i]);
					depthFirstSearchHelper(path, paths, node->children()[i], endNode);
					path.pop_back();
				}
			}
			*/
			

			/*// iterate over all children
			const int N = node->childrenNumber();
			for (int i = 0; i < N; ++i)
			{
				// check if current node is target end node or has no children
				if ((node->children()[i] == endNode) || !(node->children()[i]->hasChildren()))
				{
					// path should not be saved to paths if an endNode was specified (!= nullptr) AND the node has no children - if condition below
					// if (endNode == nullptr || node->children()[i]->hasChildren())
					// if (!(endNode != nullptr && !node->children()[i]->hasChildren()))
					if ((endNode == nullptr && !(node->children()[i]->hasChildren())) || endNode != nullptr)
					{
						// save current path to paths
						std::vector<Node<T, U>*> tmpPath = path;
						tmpPath.push_back(node->children()[i]);
						paths.push_back(tmpPath);
					}
				}
				else
				{
					// check if current node is not in the current path
					if (std::find(path.begin(), path.end(), node->children()[i]) == path.end())
					{
						path.push_back(node->children()[i]);
						depthFirstSearchHelper(path, paths, node->children()[i], endNode);
						path.pop_back();
					}
				}
			}*/

			/*
				Stack connectionPath = new Stack();					// std::vector<Node<T, U>*>
				List<Stack> connectionPaths = new ArrayList<>();	// std::list<std::vector<Node<T, U>*>>

				// Push to connectionsPath the object that would be passed as the parameter 'node' into the method below
				void findAllPaths(Object node, Object targetNode)
				{
					for (Object nextNode : nextNodes(node))
					{
					   if (nextNode.equals(targetNode))
					   {
						   Stack temp = new Stack();
						   for (Object node1 : connectionPath)
							   temp.add(node1);
						   connectionPaths.add(temp);
					   }
					   else if (!connectionPath.contains(nextNode))
					   {
						   connectionPath.push(nextNode);
						   findAllPaths(nextNode, targetNode);
						   connectionPath.pop();
						}
					}
				}
				*/
		}
	};

	#pragma endregion Graph
}