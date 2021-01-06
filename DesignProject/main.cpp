// Visual Studio 2019

/*
USAGE NOTES:
- Node<T, U>: T is the class type associated to each node (Instrument, Animal, City, ...), U is the value type of each edge (int, double, ...).
- Type U must be orderable, that is, it must have operators <, <=, >, =>.		
*/

#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <QSplashScreen>

// GRAPH ALGORITHMS TESTING 
/*
#include <chrono>
#include <QDebug>
#include <ctime>

using namespace GraphSearch;

struct Int {
	private:
		int _value;
	public:
		Int(int value) { _value = value; }
		int value() const { return _value; }
};
typedef Node<Int, int> IntNode;
typedef Graph<Int, int> IntTree;

void createBigAssTree(IntNode* node, int childrenPerNode, int levels)
{
	if (levels > 0)
	{
		for (int i = 0; i < childrenPerNode; ++i)
		{
			Int* a = new Int(rand());
			node->appendChild(new IntNode(a), a->value());
		}

		for (auto it = node->children().begin(); it != node->children().end(); ++it)
			createBigAssTree(it->first, childrenPerNode, levels - 1);
	}
}

// create big-ass tree
std::srand(std::time(0));
IntNode root;
createBigAssTree(&root, 3, 3);

IntTree tree(root);
std::vector<IntNode*> v = tree.greedySearch();
qDebug() << "GREEDY SEARCH";
QString s;
for (int i = 0; i < v.size() - 1; ++i)
{
	if (v[i]->object() == nullptr)
		s += "R";
	else
		s += QString::number(v[i]->object()->value());
	s +=  "  ->  ";
}
qDebug() << s + QString::number(v[v.size() - 1]->object()->value());
s.clear();

std::list<std::vector<IntNode*>> l;
auto start = std::chrono::high_resolution_clock::now();
tree.depthFirstSearch(l);
auto end = std::chrono::high_resolution_clock::now();
qDebug() << "\nDFS SEARCH";
qDebug() << std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count() << "seconds -" << l.size() << "paths found";
auto endit = l.end();
for (auto it = l.begin(); it != endit; ++it)
{
	std::vector<IntNode*> p = *it;
	QString s;
	for (int i = 0; i < p.size() - 1; ++i)
	{
		if (p[i]->object() == nullptr)
			s += "R";
		else
			s += QString::number(p[i]->object()->value());
		s += "  ->  ";
	}
	qDebug() << s + QString::number(p[p.size() - 1]->object()->value());
}

start = std::chrono::high_resolution_clock::now();
tree.breadthFirstSearch(l);
end = std::chrono::high_resolution_clock::now();
qDebug() << "\nBFS SEARCH";
qDebug() << std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count() << "seconds -" << l.size() << "paths found";
endit = l.end();
for (auto it = l.begin(); it != endit; ++it)
{
	std::vector<IntNode*> p = *it;
	QString s;
	for (int i = 0; i < p.size() - 1; ++i)
	{
		if (p[i]->object() == nullptr)
			s += "R";
		else
			s += QString::number(p[i]->object()->value());
		s += "  ->  ";
	}
	qDebug() << s + QString::number(p[p.size() - 1]->object()->value());
}

v = tree.findShortestPath();
qDebug() << "\nSHORTEST PATH";
for (int i = 0; i < v.size() - 1; ++i)
{
	if (v[i]->object() == nullptr)
		s += "R";
	else
		s += QString::number(v[i]->object()->value());
	s += "  ->  ";
}
qDebug() << s + QString::number(v[v.size() - 1]->object()->value());
s.clear();

v = tree.findMinimumCostPath();
qDebug() << "\nMINIMUM PATH";
for (int i = 0; i < v.size() - 1; ++i)
{
	if (v[i]->object() == nullptr)
		s += "R";
	else
		s += QString::number(v[i]->object()->value());
	s += "  ->  ";
}
qDebug() << s + QString::number(v[v.size() - 1]->object()->value());
s.clear();

return 0;
*/

QSplashScreen* createLoadingScreen()
{
	QPixmap pixmap(":/MainWindow/Resources/central_img_red.png");
	pixmap = pixmap.scaledToHeight(500, Qt::TransformationMode::SmoothTransformation);
	QSplashScreen* screen = new QSplashScreen(pixmap);
	return screen;
}

int main(int argc, char *argv[])
{
	/*
	// TREE of INSTRUMENTS
	QList<Category*> categories;
	QString err;
	readJsonBD("database.json", &categories, &err);

	auto g = createGraphWithFilters(&categories, false, Instrument::Type::UX,
		Instrument::Design::DesignNotSpecified, Instrument::Approach::Creative, 1, &Instrument::tmax);

	QFile saveFile("results.txt");
	if (saveFile.open(QIODevice::WriteOnly))
	{
		saveFile.write(printInstrumentsGraph(g).toUtf8());
		saveFile.close();
	}
	else
	{
		qDebug() << "could not save file";
		return false;
	}

	return 0; */

	QApplication a(argc, argv);

	// create loading screen
	QSplashScreen* screen = createLoadingScreen();
	screen->show();
	a.processEvents();
	QThread::sleep(2);

	// create main window
	MainWindow w;
	w.show();
	screen->finish(&w);
	return a.exec();
}