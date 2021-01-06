#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <set>
#include <vector>
#include "GraphSearch.h"


namespace DesignMethodToolKit
{
	// classes
	class Instrument : public QObject
	{
		Q_OBJECT;	

	public:
		// public enum classes
		enum class Design { UCD, SD, GD, DesignNotSpecified };		
		enum class Approach { Creative, Analytic, ApproachNotSpecified };
		enum class Type { Product, Service, UX, All };

		// constructors
		Instrument(QObject* parent = Q_NULLPTR) : _parent(parent), QObject(parent) { }
		/* Instrument(const QString& name, const QString& description, bool istargeted,
			Design design, Approach approach, Type type,
			int category, int subcategory, 
			const QStringList& postrelations, double tmin, double tmax,
			QObject* parent = Q_NULLPTR);*/
		Instrument(const QString& name, const QString& description, bool istargeted,
			Design design, Approach approach, Type type,
			int category, int subcategory,
			double tmin, double tmax,
			QObject* parent = Q_NULLPTR);

		// properties getters
		QString name() const { return _name; }
		QString description() const { return _description; }
		bool istargeted() const { return _istargeted; }
		Design design() const { return _design; }
		Approach approach() const { return _approach; }
		Type type() const { return _type; }
		int category() const { return _category; }
		int subcategory() const { return _subcategory; }
		// const QStringList& postrelations() const { return _postrelations; }
		double tmin() const { return _tmin; }
		double tmax() const { return _tmax; }
		double tavg() const { return ((_tmax - _tmin) / 2.0); }
		std::pair<double, double> timeRange() const { return std::make_pair(_tmin, _tmax); }
		bool isExcluded() const { return _excluded; }
		void setExcluded(bool flag) { _excluded = flag; }
		void validate() const;

		// json api methods
		void write(QJsonObject& json) const;
		void read(const QJsonObject& json);

		// static methods
		static QString designToString(Instrument::Design design)
		{
			switch (design)
			{
			case Instrument::Design::UCD:
				return "UCD";
			case Instrument::Design::GD:
				return "GD";
			case Instrument::Design::SD:
				return "SD";
			case Instrument::Design::DesignNotSpecified:
				return "DesignNotSpecified";
			default:
				throw std::invalid_argument("invalid design");
			}
		}
		static QString approachToString(Instrument::Approach approach)
		{
			switch (approach)
			{
			case Instrument::Approach::Analytic:
				return "Analytic";
			case Instrument::Approach::Creative:
				return "Creative";
			case Instrument::Approach::ApproachNotSpecified:
				return "ApproachNotSpecified";
			default:
				throw std::invalid_argument("invalid approach");
			}
		}
		static QString typeToString(Instrument::Type type)
		{
			switch (type)
			{
			case Instrument::Type::Product:
				return "Product";
			case Instrument::Type::Service:
				return "Service";
			case Instrument::Type::UX:
				return "UX";
			case Instrument::Type::All:
				return "All";
			default:
				throw std::invalid_argument("invalid type");
			}
		}
		static Instrument::Design stringToDesign(const QString& s)
		{
			if (QString::compare(s, "UCD", Qt::CaseInsensitive) == 0)
				return Instrument::Design::UCD;
			else if (QString::compare(s, "GD", Qt::CaseInsensitive) == 0)
				return Instrument::Design::GD;
			else if (QString::compare(s, "SD", Qt::CaseInsensitive) == 0)
				return Instrument::Design::SD;
			else if (QString::compare(s, "DesignNotSpecified", Qt::CaseInsensitive) == 0)
				return Instrument::Design::DesignNotSpecified;
			else
				throw std::invalid_argument("invalid design");
		}
		static Instrument::Approach stringToApproach(const QString& s)
		{
			if (QString::compare(s, "Analytic", Qt::CaseInsensitive) == 0)
				return Instrument::Approach::Analytic;
			else if (QString::compare(s, "Creative", Qt::CaseInsensitive) == 0)
				return Instrument::Approach::Creative;
			else if (QString::compare(s, "ApproachNotSpecified", Qt::CaseInsensitive) == 0)
				return Instrument::Approach::ApproachNotSpecified;
			else
				throw std::invalid_argument("invalid approach");
		}
		static Instrument::Type stringToType(const QString& s)
		{
			if (QString::compare(s, "Product", Qt::CaseInsensitive) == 0)
				return Instrument::Type::Product;
			else if (QString::compare(s, "Service", Qt::CaseInsensitive) == 0)
				return Instrument::Type::Service;
			else if (QString::compare(s, "UX", Qt::CaseInsensitive) == 0)
				return Instrument::Type::UX;
			else if (QString::compare(s, "All", Qt::CaseInsensitive) == 0)
				return Instrument::Type::All;
			else
				throw std::invalid_argument("invalid type");
		}

	private:	
		// private members
		QObject* _parent;
		QString _name;
		QString _description;
		bool _istargeted;
		Design _design;
		Approach _approach;
		Type _type;
		int _category;
		int _subcategory;
		// QStringList _postrelations;
		double _tmin;
		double _tmax;
		bool _excluded;
	};



	class Category : public QObject
	{
		Q_OBJECT;

	public:
		// constructors
		Category(QObject* parent = Q_NULLPTR) : _parent(parent), QObject(parent) { }
		Category(int idnumber, QObject* parent = Q_NULLPTR);
		Category(const Category& category);

		// public api methods
		int idNumber() const { return _idnumber; }
		bool hasInstruments() const { return !_instruments.isEmpty(); }
		void addInstrument(Instrument* const instrument);
		const QList<Instrument*>& instruments() const { return _instruments; }
		QList<Instrument*> instrumentsInSubcategory(int idx) const;
		bool hasSubcategories() const { return !_subcategories.empty(); }
		QList<int> subcategories() const;	// this list is not returned as reference because it is itself a copy
		void validate() const;
		
		// public filtering methods
		QList<Instrument*> applyTargetFilter(const QList<Instrument*>& availableInstruments, bool istargeted) const;
		QList<Instrument*> applyTypeFilter(const QList<Instrument*>& availableInstruments, Instrument::Type type) const;
		QList<Instrument*> applyDesignFilter(const QList<Instrument*>& availableInstruments, Instrument::Design design) const;
		QList<Instrument*> applyApproachFilter(const QList<Instrument*>& availableInstruments, Instrument::Approach approach) const;
		QList<Instrument*> applyFilters(bool istargeted, Instrument::Type type, Instrument::Design design, Instrument::Approach approach) const;
		QList<Instrument*> applyFiltersToSubcategory(int subcategory, bool istargeted, Instrument::Type type, Instrument::Design design, Instrument::Approach approach) const;

		// json api methods
		void write(QJsonObject& json) const;
		void read(const QJsonObject& json);

	private:
		// private members
		QObject* _parent;
		int _idnumber;
		QList<Instrument*> _instruments;
		std::set<int> _subcategories;		// every time an instrument is added to the category, an int with its subcategory is inserted. 
	};


	
	class Method : public QObject
	{
		Q_OBJECT;

	public:
		// constructors
		Method(QObject* parent = Q_NULLPTR) : _parent(parent), QObject(parent) { }
		Method(const Method& method);
		Method(const std::vector<Instrument*>& instruments, QObject* parent = Q_NULLPTR);
		Method(const std::vector<GraphSearch::Node<Instrument, double>*>& instrumentsNodes, QObject* parent = Q_NULLPTR);

		// public api methods
		const std::vector<Instrument*>& instruments() const { return _instruments; }
		const QList<Instrument*> instrumentsToList() const;
		const Instrument* instrumentAt(int idx) const { return _instruments[idx]; }
		void appendInstrument(Instrument* instrument) { _instruments.push_back(instrument); }
		std::pair<double, double> totalTimeRange();

		// operator overloading
		Method& operator=(const Method& method);

	private:
		// private members
		QObject* _parent;
		std::vector<Instrument*> _instruments;
	};



	// type definitions
	typedef GraphSearch::Node<Instrument, double> InstrumentNode;
	typedef GraphSearch::Graph<Instrument, double> InstrumentsTree;



	// static functions
	static QString jsonParsingErrorToString(QJsonParseError::ParseError error)
	{
		switch (error)
		{
		case QJsonParseError::NoError:
			return "NoError";
		case QJsonParseError::UnterminatedObject:
			return "UnterminatedObject";
		case QJsonParseError::MissingNameSeparator:
			return "MissingNameSeparator";
		case QJsonParseError::UnterminatedArray:
			return "UnterminatedArray";
		case QJsonParseError::MissingValueSeparator:
			return "MissingValueSeparator";
		case QJsonParseError::IllegalValue:
			return "IllegalValue";
		case QJsonParseError::TerminationByNumber:
			return "TerminationByNumber";
		case QJsonParseError::IllegalNumber:
			return "IllegalNumber";
		case QJsonParseError::IllegalEscapeSequence:
			return "IllegalEscapeSequence";
		case QJsonParseError::IllegalUTF8String:
			return "IllegalUTF8String";
		case QJsonParseError::UnterminatedString:
			return "UnterminatedString";
		case QJsonParseError::MissingObject:
			return "MissingObject";
		case QJsonParseError::DeepNesting:
			return "DeepNesting";
		case QJsonParseError::DocumentTooLarge:
			return "DocumentTooLarge";
		case QJsonParseError::GarbageAtEnd:
			return "GarbageAtEnd";
		default:
			throw std::invalid_argument("invalid json parsing error");
		}
	}

	bool writeJsonDB(const QString& filename, const QList<Category*>& categoryList, QString* error);

	bool readJsonBD(const QString& filename, QList<Category*>* categoriesList, QString* error);

	InstrumentsTree createGraphWithFilters(const QList<Category*>* categoriesList, const bool istargeted,
		const Instrument::Type type, const Instrument::Design design, const Instrument::Approach approach, int time, double(Instrument::* method)() const);

	QString methodsToString(const std::list<std::vector<InstrumentNode*>>& paths);
	
	QString printInstrumentsGraph(InstrumentsTree& graph);
}