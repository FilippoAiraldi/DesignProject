#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QThread>
#include "DesignMethodToolKit.h"
#include <QMessageBox>
#include <QStackedLayout>



using namespace DesignMethodToolKit;
class WaitingSpinnerWidget;
class QMessageBox;



static const QStringList questions = QStringList({ 
	"Do you have a target concerning the development phase?",
	"Do you know the output type of your final result?",
	"Which of these definitions, written in \"Designing for interaction\"\nby Dan Saffer, do you feel closest to your design phase?",
	"Which of these approaches do you feel closest to your design phase?",
	"Are you looking for a fast development or an in-depth development?"
	});

static const std::vector<QStringList> answers { 
	QStringList({ "Yes", "No" }),
	QStringList({ "Product", "Service", "UX", "None of these" }),
	QStringList({ "User Center Design\n\n\"The philosophy behind is: users know best.\n"
					"[..] designers view users as co-workers. [..]\n"
					"involve users in every stage of the project [..]\n"
					"to focus on the needs and goals of the users.\"\n\n\n", 
				  "System Design\n\n\"The center of the design process [..] is a set\n"
					"of entities that act upon each other. [..] is a\n"
					"structured, rigorous design approach that is\n"
					"excellent for tackling complex problems and\n"
					"offers a holistic approach to designing. [..]\n"
					"focus on the whole context [..] in which a\n"
					"product or servicce will be used.\"", 
				  "Genius Design\n\n\"Designers use their best judgement as to\n"
					"what users want and then design the product\n"
					"based on that judgement. User involvement, if\n"
					"it occurs at all, comes at the end of the\n"
					"process. [..] the success rests on the skill of\n"
					"the designer [..] is propably best practiced by\n"
					"experienced designers.\"", 
				  "\n\n\n\nNone of these\n\n\n\n" }),
	QStringList({ "Creative Approach", "Analytic Approach", "None of these" }), 
	QStringList({ "Very Fast", "Fast", "In-depth", "Very In-depth" })
	};



class RoundedButton : public QPushButton
{
	Q_OBJECT

public:
	RoundedButton(const QString& text, QWidget* parent = Q_NULLPTR) : QPushButton(text, parent)
	{
		setStyleSheet("QPushButton {  border: 1px solid white; background-color: #EDEDED; "
			"color: black; font: 20px; "
			"text-align: center; padding: 9px; border-radius: 22px; }"
			"QPushButton:hover { background-color: #C6C6C6; }"
			"QPushButton:pressed { background-color: #9D9D9C; }");
	}
};



class AnswersWidget : public QWidget
{
	Q_OBJECT

public:
	AnswersWidget(QWidget* parent = Q_NULLPTR) : QWidget(parent)
	{
		QHBoxLayout* hLayout = new QHBoxLayout(this);
	}
	void addAnswers(const QStringList& answers)
	{
		QHBoxLayout* hLayout = qobject_cast<QHBoxLayout*>(layout());

		for (int i = 0; i < answers.size() - 1; ++i)
		{
			RoundedButton* btn = new RoundedButton(answers[i], this);
			_answersButtons.push_back(btn);
			btn->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Maximum);
			connect(btn, &RoundedButton::clicked, this, &AnswersWidget::onButtonClicked);
			
			hLayout->addWidget(btn, 0, Qt::AlignVCenter);
			hLayout->addSpacing(50);
		}
		
		RoundedButton* btn = new RoundedButton(answers.back(), this);
		_answersButtons.push_back(btn);
		btn->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Maximum);
		connect(btn, &RoundedButton::clicked, this, &AnswersWidget::onButtonClicked);

		hLayout->addWidget(btn, 0, Qt::AlignVCenter);
	}
	void clearAnswers()
	{
		for (int i = 0; i < _answersButtons.size(); ++i)
		{
			_answersButtons[i]->disconnect();
			_answersButtons[i]->deleteLater();
		}	
		_answersButtons.clear();

		while (layout()->takeAt(0)) 
			delete layout()->takeAt(0);
	}

signals:
	void answerGiven(int idx);

private:
	std::vector<RoundedButton*> _answersButtons;

	void onButtonClicked(bool checked)
	{
		for (int i = 0; i < _answersButtons.size(); ++i)
		{
			if (sender() == _answersButtons[i])
			{
				emit answerGiven(i);
				break;
			}
		}
	}
};



class WorkerThread : public QThread
{
	Q_OBJECT
		
public:
	WorkerThread(QObject* parent = nullptr);
	void specifyInputs(const std::vector<int>& inputs);

protected:
	void run() override;

private:
	bool _istargeted;
	Instrument::Type _type;
	Instrument::Design _design;
	Instrument::Approach _approach;
	int _time;

signals:
	void categoriesLoaded(const QList<Category*>& categories);
	void resultsReady(const Method& method, const QString& errMsg);
};



class CategoryButton : public QPushButton
{
	Q_OBJECT

public:
	CategoryButton(Instrument* instrument, bool highlighted, QWidget* parent = Q_NULLPTR) : QPushButton(parent)
	{
		this->instrument = instrument;
		this->highlighted = highlighted;

		this->setFixedSize(75, 75 / 1.618);
		if (highlighted)
			this->setStyleSheet("QPushButton { border: 1px solid white; background-color: red; border-radius: 3px; } "
				"QPushButton:hover { background-color: #D60000; } QPushButton:pressed { background-color: #B00000; }");
		else
			this->setStyleSheet("QPushButton { border: 1px solid white; background-color: #EDEDED; border-radius: 3px; } "
				"QPushButton:hover { background-color: #C6C6C6; } QPushButton:pressed { background-color: #9D9D9C; }");
	}
	Instrument* instrument;
	bool highlighted;
};



class CategoryWidget : public QWidget
{
	Q_OBJECT

public:
	CategoryWidget(Category* category, const QList<Instrument*>& chosenInstruments, bool isNumberUp, QWidget* parent = Q_NULLPTR);

private:
	QWidget* _parent;
	Category* _category;
	QList<Instrument*> _chosenInstruments;
	bool _isnumberup;
	
private slots:
	void showInstrumentInfo();
};



class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	static void showMessageBox(QMessageBox::Icon ico, const QString& text, const QString& informativeText);

private:
	Ui::MainWindowClass ui;
	QStackedLayout* _stackedLayout;
	QLabel* _questionLabel;
	AnswersWidget* _answersWidget;
	WaitingSpinnerWidget* _spinner;
	std::vector<int> _answersGiven;
	QLabel* _cover;
	static int questionCnt;
	QPropertyAnimation* _fadingInAnimation;
	QPropertyAnimation* _fadingOutAnimation;
	QList<Category*> _categories;
	Method _method;

	QWidget* createQuestionsAndAnswersWidget();
	QWidget* createCategoriesAndMethodWidget();

private slots:
	void onAnswerReceived(int idx);
	void displayNextQuestion();
	void startComputations();
	void onResultsReady(const Method& method, const QString& errMsg);
	void displayResults();
};