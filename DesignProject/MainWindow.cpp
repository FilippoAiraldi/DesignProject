#include "MainWindow.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPixMap>
#include <QGraphicsOpacityEffect>
#include <QGraphicsBlurEffect>
#include "WaitingSpinnerWidget.h"
#include <QFile>
#include <QFrame>
#include <QDialog>



// ------------------------------------------------------------------------------------



WorkerThread::WorkerThread(QObject* parent) : QThread(parent) { }

void WorkerThread::specifyInputs(const std::vector<int>& inputs)
{
	if (inputs.size() != 5)
		throw std::invalid_argument("inputs dimensions incorrect");

	_istargeted = !static_cast<bool>(inputs.at(0));
	_type = static_cast<Instrument::Type>(inputs.at(1));
	_design = static_cast<Instrument::Design>(inputs.at(2));
	_approach = static_cast<Instrument::Approach>(inputs.at(3));
	_time = inputs.at(4) + 1;
}

void WorkerThread::run()	
{
	qRegisterMetaType<Method>("Method"); // so it can be sent through signals
	qRegisterMetaType<QList<Category*>>("QList<Category*>"); 

	// load instruments json database
	QList<Category*> categories;
	QString err;
	if (!readJsonBD("database.json", &categories, &err))
	{
		emit resultsReady(Method(), QStringLiteral("Error while reading instruments database: ") + err + ".");
		return;
	}
	emit categoriesLoaded(categories);
	
	// build instruments tree
	InstrumentsTree t = createGraphWithFilters(&categories, _istargeted, _type, _design, _approach, _time, &Instrument::tmax);

	// find all methods
	std::list<std::vector<InstrumentNode*>> methods;
	t.depthFirstSearch(methods);	// try shortest, minimumcost and greedy algo

	// extract best method
	std::list<std::vector<InstrumentNode*>>::iterator bestIt = methods.begin();
	double bestVal = Method(*bestIt, this).totalTimeRange().second;
	std::list<std::vector<InstrumentNode*>>::iterator end = methods.end();
	for (std::list<std::vector<InstrumentNode*>>::iterator it = std::next(bestIt, 1); it != end; ++it)
	{
		Method method(*it, this);
		double val = method.totalTimeRange().second;
		if (val < bestVal)
		{
			bestIt = it;
			bestVal = val;
		}
	}

	// save all paths into a file .txt
	QFile saveFile("results.txt");
	if (saveFile.open(QIODevice::WriteOnly))
	{
		// write headers
		QString text = QString("INPUTS: %1, %2, %3, %4, %5\r\n").arg(_istargeted ? "targeted" : "not-targeted").arg(Instrument::typeToString(_type))
			.arg(Instrument::designToString(_design)).arg(Instrument::approachToString(_approach)).arg(_time);
		text += "PATHS FOUND: " + QString::number(methods.size()) + "\r\n";
		text += QString("BEST PATH: N. %1\r\n\r\n").arg(std::distance(methods.begin(), bestIt) + 1);

		// write all methods
		text += methodsToString(methods);
		
		// write to file
		saveFile.write(text.toUtf8());
		saveFile.close();
	}

	// emit best method as result
	emit resultsReady(Method(*bestIt, this), QString());
}



// ------------------------------------------------------------------------------------



CategoryWidget::CategoryWidget(Category* category, const QList<Instrument*>& chosenInstruments, bool isNumberUp, QWidget* parent) : QWidget(parent)
{
	_parent = parent;
	_category = category;
	_isnumberup = isNumberUp;

	// extract from the chosen instruments list the ones belonging to this category
	for (int i = 0; i < chosenInstruments.size(); ++i)
		if (chosenInstruments[i] != NULL && chosenInstruments[i]->category() == _category->idNumber()) // there's always the null root to jump...
			_chosenInstruments.append(chosenInstruments[i]);

	// create number and frame
	QVBoxLayout* vLayout = new QVBoxLayout(this);
	setLayout(vLayout);
	vLayout->setSpacing(10);
	QFont f("Avenir", 10, QFont::Weight::ExtraLight);
	QLabel* numberLabel = new QLabel(QString::number(_category->idNumber()), this);
	numberLabel->setAlignment(Qt::AlignCenter);
	numberLabel->setFont(f);
	numberLabel->setStyleSheet("QLabel { color: #8A8A8A; }");
	QFrame* frame = new QFrame(this);
	frame->setStyleSheet("QFrame { background-color: transparent; border: 1px solid white; border-radius: 10px; }");
	frame->setContentsMargins(10, 10, 10, 10);
	
	// add frame and number to layout
	if (isNumberUp)
	{
		vLayout->addWidget(numberLabel, 0);
		vLayout->addWidget(frame, 1);
	}
	else
	{
		vLayout->addWidget(frame, 1);
		vLayout->addWidget(numberLabel, 0);
	}

	// create subcategories columns inside frame, each hosting buttons as instruments
	int spacing = 24;
	QHBoxLayout* hLayout = new QHBoxLayout(frame);
	frame->setLayout(hLayout);
	for (int i = 0; i < _category->subcategories().size(); ++i)
	{
		// create column and add to layout
		QVBoxLayout* vCol = new QVBoxLayout(frame);
		vCol->setSpacing(spacing);
		hLayout->addLayout(vCol);

		// get instruments in this subcategory and add them to column
		const QList<Instrument*> instrumentsInSubcategory = _category->instrumentsInSubcategory(_category->subcategories()[i]);
		const int N = instrumentsInSubcategory.size();
		for (int j = 0; j < N; ++j)
		{
			// create button; display instrument info when button clicked
			Instrument* instr = instrumentsInSubcategory[j];
			CategoryButton* btn = new CategoryButton(instr, _chosenInstruments.contains(instr), frame);
			vCol->addWidget(btn, 0, isNumberUp ? Qt::AlignTop : Qt::AlignBottom);
			connect(btn, &QPushButton::clicked, this, &CategoryWidget::showInstrumentInfo);

			// if columns contains more than four buttons, create new one
			if ((j + 1) % 4 == 0 && N > 4)
			{
				vCol->insertStretch(isNumberUp ? 4 : 0, 1);
				vCol = new QVBoxLayout(frame);
				vCol->setSpacing(spacing);
				hLayout->addSpacing(spacing / 1.5);
				hLayout->addLayout(vCol);
			}
		}
		vCol->insertStretch(isNumberUp ? 4 : 0, 1);

		// add spacing
		if (i != _category->subcategories().size() - 1)
			hLayout->addSpacing(spacing * 2);
	}
}

void CategoryWidget::showInstrumentInfo()
{
	// cast sender into button
	CategoryButton* btn = qobject_cast<CategoryButton*>(sender());

	// apply blur effect
	QGraphicsBlurEffect* effect = new QGraphicsBlurEffect(_parent);
	effect->setBlurHints(QGraphicsBlurEffect::PerformanceHint);
	effect->setBlurRadius(4);
	_parent->setGraphicsEffect(effect);

	// create a popup dialog
	QDialog* dialog = new QDialog(_parent, Qt::Popup);
	QVBoxLayout* vLayout = new QVBoxLayout(dialog);
	dialog->setLayout(vLayout);
	dialog->setStyleSheet("background-color: #C6C6C6 ; color: black;");
	dialog->setContentsMargins(10, 10, 10, 10);
	dialog->setMinimumSize(300, 300);
	connect(dialog, &QDialog::finished, [this, btn] { _parent->setGraphicsEffect(0); btn->setAttribute(Qt::WA_UnderMouse, false); });

	// write name, logo, time, and description
	QFont f("Avenir", 12, QFont::Weight::Bold);
	QLabel* name = new QLabel(btn->instrument->name(), dialog);
	name->setAlignment(Qt::AlignCenter);
	name->setFont(f);
	vLayout->addWidget(name, Qt::AlignHCenter);
	vLayout->addSpacing(30);
	QPixmap pixmap(QString(":/MainWindow/Resources/central_img%1.png").arg(btn->highlighted ? "_red" : "_dark"));
	pixmap = pixmap.scaledToHeight(130, Qt::TransformationMode::SmoothTransformation);
	QLabel* logo = new QLabel(dialog);
	logo->setPixmap(pixmap);
	logo->setAlignment(Qt::AlignHCenter);
	vLayout->addWidget(logo);
	vLayout->addSpacing(30);
	QLabel* time = new QLabel(dialog);
	time->setText(QString("%1 - %2 hours").arg(btn->instrument->tmin()).arg(btn->instrument->tmax()));
	time->setAlignment(Qt::AlignCenter);
	f = time->font();
	f.setPointSize(11);
	time->setFont(f);
	vLayout->addWidget(time, Qt::AlignHCenter);
	vLayout->addSpacing(15);
	QLabel* description = new QLabel(btn->instrument->description(), dialog);
	description->setAlignment(Qt::AlignCenter);
	f.setPointSize(10);
	description->setFont(f);
	description->setWordWrap(true);
	vLayout->addWidget(description, Qt::AlignHCenter);
	
	// show dialog
	qApp->processEvents();
	dialog->show(); // or open()
}


// ------------------------------------------------------------------------------------



int MainWindow::questionCnt = 0;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	ui.setupUi(this);
	setMenuBar(nullptr);				// cancel the menu bar
	removeToolBar(ui.mainToolBar);		// cancel the tool bar

	// set mainwindow style
	setStyleSheet("QMainWindow { background: #3C3C3B; color: white; }");
	setWindowTitle(QStringLiteral("Methodological Path Finder"));
	setWindowIcon(QIcon(":/MainWindow/Resources/icon.png"));
	setWindowState(Qt::WindowState::WindowMaximized);

	// create grid layout that contains everything, namely the stacked layout and the cover
	QGridLayout* grid = new QGridLayout(this);
	grid->setRowStretch(0, 1);
	grid->setColumnStretch(0, 1);
	centralWidget()->setLayout(grid);
	
	// create stackedLayout
	_stackedLayout = new QStackedLayout(grid);
	_stackedLayout->addWidget(createQuestionsAndAnswersWidget());
	_stackedLayout->setCurrentIndex(0);

	// create cover
	QPixmap pixmap(":/MainWindow/Resources/central_img.png");
	pixmap = pixmap.scaledToHeight(300, Qt::TransformationMode::SmoothTransformation);
	_cover = new QLabel(this);
	_cover->setStyleSheet("QLabel { background: #3C3C3B; }");
	_cover->setSizePolicy(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::MinimumExpanding);
	_cover->setPixmap(pixmap);
	_cover->setAlignment(Qt::AlignCenter);
	grid->addWidget(_cover, 0, 0);
	QGraphicsOpacityEffect* eff = new QGraphicsOpacityEffect(_cover);
	_cover->setGraphicsEffect(eff);

	// create fade out/in animations for the cover
	int vel = 600;
	_fadingInAnimation = new QPropertyAnimation(_cover->graphicsEffect(), "opacity");
	_fadingInAnimation->setDuration(vel);
	_fadingInAnimation->setStartValue(0.0);
	_fadingInAnimation->setEndValue(1.0);
	_fadingInAnimation->setEasingCurve(QEasingCurve::Linear);
	connect(_fadingInAnimation, &QPropertyAnimation::finished, this, &MainWindow::displayNextQuestion);
	_fadingOutAnimation = new QPropertyAnimation(_cover->graphicsEffect(), "opacity");
	_fadingOutAnimation->setDuration(vel);
	_fadingOutAnimation->setStartValue(1.0);
	_fadingOutAnimation->setEndValue(0.0);
	_fadingOutAnimation->setEasingCurve(QEasingCurve::Linear);
	connect(_fadingOutAnimation, &QPropertyAnimation::finished, [=] { _cover->setVisible(false); });
	_fadingOutAnimation->start(QPropertyAnimation::KeepWhenStopped);
}

QWidget* MainWindow::createQuestionsAndAnswersWidget()
{
	QWidget* widget = new QWidget(this);

	// create vertical layout
	QVBoxLayout* vLayout = new QVBoxLayout(widget);
	widget->setLayout(vLayout);

	// create question label
	QFont f("Avenir", 20, QFont::Weight::ExtraLight);
	_questionLabel = new QLabel(widget);
	_questionLabel->setAlignment(Qt::AlignCenter);
	_questionLabel->setText(questions[0]);
	_questionLabel->setFont(f);
	_questionLabel->setStyleSheet("QLabel { color: white; }");
	vLayout->addWidget(_questionLabel);

	// create central image
	QPixmap pixmap(":/MainWindow/Resources/central_img.png");
	pixmap = pixmap.scaledToHeight(300, Qt::TransformationMode::SmoothTransformation);
	QLabel* logo = new QLabel(widget);
	logo->setPixmap(pixmap);
	logo->setAlignment(Qt::AlignCenter);
	vLayout->addWidget(logo);

	// create answers widget
	_answersWidget = new AnswersWidget(widget);
	vLayout->addWidget(_answersWidget);
	_answersWidget->addAnswers(answers[questionCnt]);
	connect(_answersWidget, &AnswersWidget::answerGiven, this, &MainWindow::onAnswerReceived);

	return widget;
}

QWidget* MainWindow::createCategoriesAndMethodWidget()
{
	QWidget* widget = new QWidget(this);

	// create vertical layout and the two horizontal layouts
	QVBoxLayout* vLayout = new QVBoxLayout(this);
	widget->setLayout(vLayout);
	QHBoxLayout* hLayoutUp = new QHBoxLayout(this);
	QHBoxLayout* hLayoutDown = new QHBoxLayout(this);
	vLayout->addLayout(hLayoutUp, 1);
	vLayout->addLayout(hLayoutDown, 1);

	// insert categories inside layouts
	hLayoutUp->addStretch(3);
	hLayoutDown->addStretch(3);
	for (int i = 0; i < _categories.size(); ++i)
		(i < 5 ? hLayoutUp : hLayoutDown)->addWidget(new CategoryWidget(_categories[i], _method.instrumentsToList(), (i < 5), this), 1);
	hLayoutUp->addStretch(3);
	hLayoutDown->addStretch(3);
	return widget;
}

void MainWindow::showMessageBox(QMessageBox::Icon ico, const QString& text, const QString& informativeText)
{
	QMessageBox msg;
	msg.setIcon(ico);
	msg.setWindowTitle(QStringLiteral("Methodological Path Finder"));
	msg.setWindowIcon(QIcon(":/MainWindow/Resources/icon.png"));
	msg.setText(QString("<html><head/><body><p><b>%1<b></p></body></html>").arg(text));
	msg.setInformativeText(informativeText);
	msg.setStandardButtons(QMessageBox::Ok);
	msg.setDefaultButton(QMessageBox::Ok);
	// msg.setStyleSheet("QMessageBox { color: black; }");
	msg.exec();
}

void MainWindow::onAnswerReceived(int idx)
{
	// start fadeout animation
	_cover->setVisible(true);
	_fadingInAnimation->start(QPropertyAnimation::KeepWhenStopped);
	
	// save idx of given answer
	_answersGiven.push_back(idx);
}

void MainWindow::displayNextQuestion()
{
	// dispaly next question
	if (++questionCnt < questions.size())
	{
		_answersWidget->clearAnswers();
		_answersWidget->addAnswers(answers[questionCnt]);
		_questionLabel->setText(questions[questionCnt]);

		qApp->processEvents();

		_fadingOutAnimation->start(QPropertyAnimation::KeepWhenStopped);
	}
	// display waiting widget, then start computations
	else
	{
		// delete answers widget
		_answersWidget->clearAnswers();

		// display waiting line
		_questionLabel->setText("We are preparing your method...");

		// display waiting widget
		_spinner = new WaitingSpinnerWidget(_stackedLayout->widget(0));
		_spinner->setRoundness(70.0);
		_spinner->setMinimumTrailOpacity(15.0);
		_spinner->setTrailFadePercentage(70.0);
		_spinner->setNumberOfLines(12);
		_spinner->setLineLength(10);
		_spinner->setLineWidth(5);
		_spinner->setInnerRadius(10);
		_spinner->setRevolutionsPerSecond(1);
		_spinner->setColor(QColor(255, 0, 0));
		_spinner->start();
	
		// add again cover to cover everything
		layout()->addWidget(_cover);
		qApp->processEvents();

		// fade cover out; at the end of animation start computations
		_fadingOutAnimation->start(QPropertyAnimation::KeepWhenStopped);
		connect(_fadingOutAnimation, &QPropertyAnimation::finished, this, &MainWindow::startComputations);
	}
}

void MainWindow::startComputations()
{
	// perform this connection only once
	disconnect(_fadingOutAnimation, &QPropertyAnimation::finished, this, &MainWindow::startComputations);

	// carry out computations in another thread; when computations are over, remove cover, fade cover in while display animations
	WorkerThread* workerThread = new WorkerThread(this);
	connect(workerThread, &WorkerThread::resultsReady, this, &MainWindow::onResultsReady);
	connect(workerThread, &WorkerThread::finished, workerThread, &QObject::deleteLater);
	connect(workerThread, &WorkerThread::categoriesLoaded, [this](const QList<Category*>& list) { _categories = list; });
	
	// convert inputs and feed them to thread; start computations
	workerThread->specifyInputs(_answersGiven);
	workerThread->start();
}

void MainWindow::onResultsReady(const Method& method, const QString& errMsg)
{
	// check results
	if (!errMsg.isEmpty())
	{
		showMessageBox(QMessageBox::Icon::Critical, QStringLiteral("An error occurred!"), errMsg + "\n\nApplication must be restarted.");
	}

	// save results if valid
	if (errMsg.isEmpty())
	{
		if (method.instruments().empty())
			showMessageBox(QMessageBox::Icon::Critical, QStringLiteral("Sorry!"), QStringLiteral("We could not find any valid method with the given inputs..."
				"\n\nRestart application and try again."));
		else
			_method = method;
	}

	// fade in the cover and when the animation finishes display the results
	_cover->setVisible(true);
	_fadingInAnimation->disconnect();	// previous connection used to display questions
	if (errMsg.isEmpty() || _method.instruments().empty())
		connect(_fadingInAnimation, &QPropertyAnimation::finished, this, &MainWindow::displayResults);
	_fadingInAnimation->start(QPropertyAnimation::KeepWhenStopped);
}

void MainWindow::displayResults()
{
	// clear layout's previous widgets
	_spinner->deleteLater();
	_answersWidget->clearAnswers(); 
	_answersWidget->deleteLater();
	_questionLabel->deleteLater();
	_fadingInAnimation->deleteLater();
	_cover->deleteLater();
	delete _stackedLayout->takeAt(0);
	qApp->processEvents();

	// check that categories are not empty
	if (_categories.isEmpty())
	{
		showMessageBox(QMessageBox::Icon::Critical, QStringLiteral("An error occurred!"), "Could not read instruments	 categories.\n\nApplication must be restarted.");
		return;
	}

	// create categories widgets
	_stackedLayout->addWidget(createCategoriesAndMethodWidget());

	// create central image
	QPixmap pixmap(":/MainWindow/Resources/central_img.png");
	pixmap = pixmap.scaledToHeight(300, Qt::TransformationMode::SmoothTransformation);
	QLabel* logo = new QLabel(this);
	logo->setPixmap(pixmap);
	logo->setAlignment(Qt::AlignCenter);
	_stackedLayout->addWidget(logo);
	_stackedLayout->setStackingMode(QStackedLayout::StackingMode::StackAll);
	_stackedLayout->setCurrentIndex(0);

	// fade cover out
	qApp->processEvents();
	_fadingOutAnimation->start(QPropertyAnimation::DeleteWhenStopped);
}