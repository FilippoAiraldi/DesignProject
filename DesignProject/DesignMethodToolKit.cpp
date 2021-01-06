#include "DesignMethodToolKit.h"

namespace DesignMethodToolKit
{
	#pragma region 

	Instrument::Instrument(const QString& name, const QString& description, bool istargeted,
		Design design, Approach approach, Type type,
		int category, int subcategory,
		double tmin, double tmax,
		QObject* parent) :
		_parent(parent),
		_name(name),
		_description(description),
		_istargeted(istargeted),
		_design(design),
		_approach(approach),
		_type(type),
		_category(category),
		_subcategory(subcategory),
		_tmin(tmin),
		_tmax(tmax),
		_excluded(false),
		QObject(parent)
	{
		// check input data correctness
		if (_name.isEmpty())
			throw std::invalid_argument("empty name");
		if (_description.isEmpty())
			throw std::invalid_argument("empty description");
		if (_category <= 0)
			throw std::invalid_argument("category must be a positive number");
		if (_subcategory <= 0)
			throw std::invalid_argument("subcategory must be a positive number");
		if (_tmin > _tmax)
			throw std::invalid_argument("invalid time range");
	}

	void Instrument::validate() const
	{
		if (_name.isEmpty())
			throw std::invalid_argument("empty name");
		if (_description.isEmpty())
			throw std::invalid_argument("empty description");
		if (_category <= 0)
			throw std::invalid_argument("category must be a positive number");
		if (_subcategory <= 0)
			throw std::invalid_argument("subcategory must be a positive number");
		if (_tmin > _tmax)
			throw std::invalid_argument("invalid time range");
	}

	void Instrument::write(QJsonObject& json) const
	{
		// write single values to json
		json["name"] = _name;
		json["description"] = _description;
		json["istargeted"] = _istargeted;
		json["design"] = designToString(_design);
		json["approach"] = approachToString(_approach);
		json["type"] = typeToString(_type);
		json["category"] = _category;
		json["subcategory"] = _subcategory;
		json["tmin"] = _tmin;
		json["tmax"] = _tmax;
		// _excluded should not be saved
	}

	void Instrument::read(const QJsonObject& json)
	{
		// load single variables
		if (json.contains("name") && json["name"].isString())
			_name = json["name"].toString();
		else
			throw std::invalid_argument("invalid json file: instrument name is invalid");
		
		if (json.contains("description") && json["description"].isString())
			_description = json["description"].toString();
		else
			throw std::invalid_argument("invalid json file: instrument description is invalid");

		if (json.contains("istargeted") && json["istargeted"].isBool())
			_istargeted = json["istargeted"].toBool();
		else
			throw std::invalid_argument("invalid json file: instrument target is invalid");

		if (json.contains("design") && json["design"].isString())
			_design = stringToDesign(json["design"].toString());
		else
			throw std::invalid_argument("invalid json file: instrument design is invalid");

		if (json.contains("approach") && json["approach"].isString())
			_approach = stringToApproach(json["approach"].toString());
		else
			throw std::invalid_argument("invalid json file: instrument approach is invalid");

		if (json.contains("type") && json["type"].isString())
			_type = stringToType(json["type"].toString());
		else
			throw std::invalid_argument("invalid json file: instrument type is invalid");

		if (json.contains("category"))
			_category = json["category"].toInt();
		else
			throw std::invalid_argument("invalid json file: instrument category is invalid");

		if (json.contains("subcategory"))
			_subcategory = json["subcategory"].toInt();
		else
			throw std::invalid_argument("invalid json file: instrument subcategory is invalid");

		if (json.contains("tmin") && json["tmin"].isDouble())
			_tmin = json["tmin"].toDouble();
		else
			throw std::invalid_argument("invalid json file: instrument tmin is invalid");

		if (json.contains("tmax") && json["tmax"].isDouble())
			_tmax = json["tmax"].toDouble();
		else
			throw std::invalid_argument("invalid json file: instrument tmax is invalid");
	}

	#pragma endregion Instrument



	#pragma region 

	Category::Category(int idnumber, QObject* parent) : _parent(parent), _idnumber(idnumber), QObject(parent)
	{
		// check input data correctness
		if (_idnumber <= 0)
			throw std::invalid_argument("category number must be a positive number");
		
		// clear list and set
		if (!_instruments.empty())
			_instruments.clear();
		if (!_subcategories.empty())
			_subcategories.clear();
	}

	Category::Category(const Category& category)
	{
		// copy parent and id
		_parent = category._parent;
		_idnumber = category._idnumber;

		// copy lists
		for (int subcategory : category._subcategories)
			_subcategories.insert(subcategory);
		for (Instrument* instrument : category._instruments)
			_instruments.append(instrument);
	}

	void Category::addInstrument(Instrument* const instrument)
	{
		// check that the instrument belongs to this category
		if (_idnumber != instrument->category())
			throw std::invalid_argument("this category and instrument category mismatch");

		// add instrument and its subcategory in the lists
		_instruments.append(instrument);
		_subcategories.insert(instrument->subcategory());
	}

	QList<Instrument*> Category::instrumentsInSubcategory(int idx) const
	{
		// get size of instruments and 
		const int N = _instruments.size();
		
		// return empty list if no instruments are present
		if (N == 0)
		{
			return QList<Instrument*>();
		}
		else
		{
			// search if requested subcategory is present in this category
			if (_subcategories.find(idx) == _subcategories.end())
			{
				return QList<Instrument*>();
			}
			else
			{
				// extract all instruments belonging to this subcategory
				QList<Instrument*> L;
				for (int i = 0; i < N; ++i)
				{
					if (_instruments[i]->subcategory() == idx)
						L.append(_instruments[i]);
				}
				return L;
			}
		}
	}

	QList<int> Category::subcategories() const
	{
		// convert set into a list of int
		QList<int> L;
		L.reserve(_subcategories.size());
		for (std::set<int>::iterator it = _subcategories.begin(); it != _subcategories.end(); ++it)
			L.append(*it);

		// check for correctness
		if (L.size() != _subcategories.size())
			throw std::bad_alloc();

		// return list of subcategories
		return L;
	}

	void Category::validate() const
	{
		if (_subcategories.size() == 1)
		{
			if (*_subcategories.begin() != 1) // if there's only one subcategory, then it must be 1
				throw std::invalid_argument("only subcategory in this category different from 1");
			else
				return;
		}
		else
		{
			// if there are more than one subcategories, they must be contiguous (1, 2, 3, ..)
			std::set<int>::iterator it = _subcategories.begin();
			for (size_t i = 0; i < _subcategories.size() - 1; ++i)
			{
				int prev = *it;
				std::advance(it, 1);
				if (*it - prev != 1)
					throw std::invalid_argument("sub-categories jump from " + std::to_string(prev) + " to " + std::to_string(*it));
			}
			return;
		}
	}

	QList<Instrument*> Category::applyTargetFilter(const QList<Instrument*>& availableInstruments, bool istargeted) const
	{
		// iterate over all instruments in this category
		QList<Instrument*> L;
		const int N = availableInstruments.size();
		for (int i = 0; i < N; ++i)
		{
			if (availableInstruments.at(i)->istargeted() == istargeted)
				L.append(availableInstruments.at(i));
		}
		return L;
	}

	QList<Instrument*> Category::applyTypeFilter(const QList<Instrument*>& availableInstruments, Instrument::Type type) const
	{
		if (type == Instrument::Type::All)
			return availableInstruments;



		// iterate over all instruments in this category
		QList<Instrument*> L;
		const int N = availableInstruments.size();
		for (int i = 0; i < N; ++i)
		{
			if (availableInstruments.at(i)->type() == type)
				L.append(availableInstruments.at(i));
		}
		return L;
	}

	QList<Instrument*> Category::applyDesignFilter(const QList<Instrument*>& availableInstruments, Instrument::Design design) const
	{
		if (design == Instrument::Design::DesignNotSpecified)
			return availableInstruments;



		// iterate over all instruments in this category
		QList<Instrument*> L;
		const int N = availableInstruments.size();
		for (int i = 0; i < N; ++i)
		{
			if (availableInstruments.at(i)->design() == design)
				L.append(availableInstruments.at(i));
		}
		return L;
	}

	QList<Instrument*> Category::applyApproachFilter(const QList<Instrument*>& availableInstruments, Instrument::Approach approach) const
	{
		if (approach == Instrument::Approach::ApproachNotSpecified)
			return availableInstruments;



		// iterate over all instruments in this category
		QList<Instrument*> L;
		const int N = availableInstruments.size();
		for (int i = 0; i < N; ++i)
		{
			if (availableInstruments.at(i)->approach() == approach)
				L.append(availableInstruments.at(i));
		}
		return L;
	}

	QList<Instrument*> Category::applyFilters(bool istargeted, Instrument::Type type, Instrument::Design design, Instrument::Approach approach) const
	{
		// ------------------ apply ISTARGETD filter ------------------
		QList<Instrument*> afterTargetedFilterInstrumentsList;
		if (istargeted)
		{
			afterTargetedFilterInstrumentsList = applyTargetFilter(_instruments, true);
			if (afterTargetedFilterInstrumentsList.size() == 1)
				return afterTargetedFilterInstrumentsList;		// return this list, no more filtering required
		}
		if (!istargeted || afterTargetedFilterInstrumentsList.empty())
		{
			afterTargetedFilterInstrumentsList = applyTargetFilter(_instruments, false);
			if (afterTargetedFilterInstrumentsList.size() == 1)
				return afterTargetedFilterInstrumentsList;
			else if (afterTargetedFilterInstrumentsList.empty())
				throw std::invalid_argument("case impossible, should never happen");
		}

		// ------------------ apply TYPE filter ------------------
		QList<Instrument*> afterTypeFilterInstrumentsList;
		if (type != Instrument::Type::All)
		{
			afterTypeFilterInstrumentsList = applyTypeFilter(afterTargetedFilterInstrumentsList, type);
			if (afterTypeFilterInstrumentsList.size() == 1)
				return afterTypeFilterInstrumentsList;
		}
		if (type == Instrument::Type::All || afterTypeFilterInstrumentsList.empty())
		{
			afterTypeFilterInstrumentsList = applyTypeFilter(afterTargetedFilterInstrumentsList, Instrument::Type::All);
			if (afterTypeFilterInstrumentsList.size() == 1)
				return afterTypeFilterInstrumentsList;
			else if (afterTypeFilterInstrumentsList.empty())
				throw std::invalid_argument("case impossible, should never happen");
		}

		// ------------------ apply DESIGN filter ------------------
		QList<Instrument*> afterDesignFilterInstrumentsList;
		if (design != Instrument::Design::DesignNotSpecified)
		{
			afterDesignFilterInstrumentsList = applyDesignFilter(afterTypeFilterInstrumentsList, design);
			if (afterDesignFilterInstrumentsList.size() == 1)
				return afterDesignFilterInstrumentsList;
		}
		if (design == Instrument::Design::DesignNotSpecified || afterDesignFilterInstrumentsList.empty())
		{
			afterDesignFilterInstrumentsList = applyDesignFilter(afterTypeFilterInstrumentsList, Instrument::Design::DesignNotSpecified);
			if (afterDesignFilterInstrumentsList.size() == 1)
				return afterDesignFilterInstrumentsList;
			else if (afterDesignFilterInstrumentsList.empty())
				throw std::invalid_argument("case impossible, should never happen");
		}

		// ------------------ apply APPPROACH filter ------------------
		QList<Instrument*> afterApproachFilterInstrumentsList;
		if (approach != Instrument::Approach::ApproachNotSpecified)
		{
			afterApproachFilterInstrumentsList = applyApproachFilter(afterDesignFilterInstrumentsList, approach);
			if (afterApproachFilterInstrumentsList.size() == 1)
				return afterApproachFilterInstrumentsList;
		}
		if (approach == Instrument::Approach::ApproachNotSpecified || afterApproachFilterInstrumentsList.empty())
		{
			afterApproachFilterInstrumentsList = applyApproachFilter(afterDesignFilterInstrumentsList, Instrument::Approach::ApproachNotSpecified);
			if (afterApproachFilterInstrumentsList.empty())
				throw std::invalid_argument("case impossible, should never happen");
		}

		// whatever the size, return list
		return afterApproachFilterInstrumentsList;
	}

	QList<Instrument*> Category::applyFiltersToSubcategory(int subcategory, bool istargeted, Instrument::Type type, Instrument::Design design, Instrument::Approach approach) const
	{
		if (_subcategories.find(subcategory) == _subcategories.end())
			throw std::invalid_argument("subcategory not existing in this category");

		// extract the instruments in given subcategory
		QList<Instrument*> instrumentsInSubcategory;
		for (int i = 0; i < _instruments.size(); ++i)
			if (_instruments.at(i)->subcategory() == subcategory)
				instrumentsInSubcategory.append(_instruments.at(i));
		if (instrumentsInSubcategory.empty())
			return QList<Instrument*>();

		// create fake category with same number and with such instruments
		Category fakeCategory(_idnumber, _parent);
		for (int i = 0; i < instrumentsInSubcategory.size(); ++i)
			fakeCategory.addInstrument(instrumentsInSubcategory.at(i));

		// apply filters to such category and return filtered instruments
		return (fakeCategory.applyFilters(istargeted, type, design, approach));
	}

	void Category::write(QJsonObject& json) const
	{
		// write single values to json
		json["idnumber"] = _idnumber;

		// write arrays to jsonarray
		QJsonArray instrumentsArray;
		for (Instrument* instrument : _instruments)
		{
			QJsonObject instrumentJson;
			instrument->write(instrumentJson);
			instrumentsArray.append(instrumentJson);
		}
		json["instruments"] = instrumentsArray;
		QJsonArray subcategoriesArray;
		for (int subcategory : _subcategories)
			subcategoriesArray.append(subcategory);
		json["subcategories"] = subcategoriesArray;
	}

	void Category::read(const QJsonObject& json)
	{
		// read single values
		if (json.contains("idnumber"))
			_idnumber = json["idnumber"].toInt();
		else
			throw std::invalid_argument("invalid json file: category id number is invalid");

		// read arrays
		if (json.contains("subcategories") && json["subcategories"].isArray())
		{
			QJsonArray subcategoriesArray = json["subcategories"].toArray();
			_subcategories.clear();
			for (int i = 0; i < subcategoriesArray.size(); ++i)
				_subcategories.insert(subcategoriesArray[i].toInt());
		}
		else
			throw std::invalid_argument("invalid json file: category subcategories are invalid");

		if (json.contains("instruments") && json["instruments"].isArray())
		{
			QJsonArray instrumentsArray = json["instruments"].toArray();
			_instruments.clear();
			_instruments.reserve(instrumentsArray.size());
			for (int i = 0; i < instrumentsArray.size(); ++i)
			{
				Instrument* instrument = new Instrument(this);
				instrument->read(instrumentsArray[i].toObject());
				_instruments.append(instrument);
			}
		}
		else
			throw std::invalid_argument("invalid json file: category instruments are invalid");
	}

	#pragma endregion Category



	#pragma region

	Method::Method(const std::vector<Instrument*>& instruments, QObject* parent) : _parent(parent), QObject(parent)
	{
		_instruments = instruments;
	}

	Method::Method(const Method& method)
	{
		_parent = method._parent;
		_instruments = method._instruments;
	}

	Method::Method(const std::vector<GraphSearch::Node<Instrument, double>*>& instrumentsNodes, QObject* parent) : _parent(parent), QObject(parent)
	{
		_instruments.clear();
		const size_t N = instrumentsNodes.size();
		_instruments.resize(N);

		for (size_t i = 0; i < N; ++i)
			_instruments[i] = instrumentsNodes[i]->object();
	}

	Method& Method::operator=(const Method& method)
	{
		if (this != &method) 
		{
			_parent = method._parent;
			_instruments = method._instruments;
		}
		return *this;
	}

	const QList<Instrument*> Method::instrumentsToList() const
	{
		QList<Instrument*> l;
		for (int i = 0; i < _instruments.size(); ++i)
			l.append(_instruments[i]);
		return l;
	}

	std::pair<double, double> Method::totalTimeRange()
	{
		double tminSum = 0.0;
		double tmaxSum = 0.0;

		const size_t N = _instruments.size();
		for (int i = 0; i < N; ++i)
		{
			if (_instruments[i] != NULL)
			{
				tminSum += _instruments[i]->tmin();
				tmaxSum += _instruments[i]->tmax();
			}
		}

		return std::make_pair(tminSum, tmaxSum);
	}

	#pragma endregion Method



	#pragma region 

	bool writeJsonDB(const QString& filename, const QList<Category*>& categoryList, QString* error)
	{
		if (filename.isEmpty() || filename.isNull())
		{
			error->append("filename empty");
			return false;
		}

		// create Json object that will host all the categories
		QJsonObject json;
		QJsonArray categoriesArray;
		for (auto c : categoryList)
		{
			QJsonObject categoryObject;
			c->write(categoryObject);
			categoriesArray.append(categoryObject);
		}
		json["categories"] = categoriesArray;

		// save as file
		QFile saveFile(filename);
		if (saveFile.open(QIODevice::WriteOnly))
		{
			QJsonDocument saveDoc(json);
			saveFile.write(saveDoc.toJson());
			error->clear();
			return true;
		}
		else
		{
			error->append("could not save file");
			return false;
		}
	}

	bool readJsonBD(const QString& filename, QList<Category*>* categoriesList, QString* error)
	{
		if (filename.isEmpty() || filename.isNull())
		{
			error->append("filename empty");
			return false;
		}

		// load file to read
		QFile loadFile(filename);
		if (loadFile.open(QIODevice::ReadOnly))
		{
			QByteArray saveData = loadFile.readAll();
			QJsonParseError err;
			QJsonDocument loadDoc(QJsonDocument::fromJson(saveData, &err));

			if (!loadDoc.isNull())
			{
				if (loadDoc.isObject())
				{
					// load as obj and extract the "categories" json array
					QJsonObject json = loadDoc.object();
					if (json.contains("categories") && json["categories"].isArray())
					{
						QJsonArray categoriesArray = json["categories"].toArray();
						categoriesList->clear();
						categoriesList->reserve(categoriesArray.size());
						for (int i = 0; i < categoriesArray.size(); ++i)
						{
							Category* category = new Category(); // without parent for now!!
							category->read(categoriesArray[i].toObject());
							categoriesList->append(category);
						}

						// validate each category and each instrument in each category
						for (int i = 0; i < categoriesList->size(); ++i)
						{
							Category* c = categoriesList->at(i);
							c->validate();
							for (int j = 0; j < c->instruments().size(); ++j)
								c->instruments().at(j)->validate();
						}
						error->clear();
						return true;
					}
					else
					{
						error->append("invalid json file: categories are invalid");
						return false;
					}
				}
				else
				{
					error->append("invalid json file: no object contained");
					return false;
				}
			}
			else
			{
				error->append("Json parsing error: " + jsonParsingErrorToString(err.error) + " at offset: " + QString::number(err.offset));
				return false;
			}
		}
		else
		{
			error->append("could not open file");
			return false;
		}
	}

	InstrumentsTree createGraphWithFilters(const QList<Category*>* categoriesList, const bool istargeted, const Instrument::Type type, const Instrument::Design design,
		const Instrument::Approach approach, int time, double(Instrument::* method)() const)
	{
		if (time < 1 || time > 4)
			throw std::invalid_argument("time must be an integer between 1 and 4 included");

		// create root and graph
		InstrumentNode* root = new InstrumentNode();
		InstrumentsTree tree(root);
		QList<InstrumentNode*> lastNodes({ root });

		// iterate over all categories
		const int N = categoriesList->size();
		for (int i = 0; i < N; ++i)
		{
			int num = categoriesList->at(i)->idNumber();

			// categories with subcategories
			if (num == 2 || num == 4 || num == 5 || num == 6 || num == 7)
			{
				// decide order in which to iterate over subcategories; based on time, decide how many subcategories to iterate over
				std::vector<int> subcat;
				int max_subcat;
				switch (num)
				{
				case 2:
				case 4:
				case 7:
					max_subcat = time < 3 ? 1 : 2;
					subcat = std::vector<int>{ 1, 2 };
					if (num == 7)
						std::reverse(subcat.begin(), subcat.end());  // 2, 1
					break;
				case 5:
					if (time == 1)
						max_subcat = 1;
					else
						max_subcat = time != 4 ? 2 : 3;
					if (type == Instrument::Type::Product || type == Instrument::Type::All)
						subcat = std::vector<int>{ 1, 3, 2 };
					else // UX, Service
						subcat = std::vector<int>{ 2, 3, 1 };
					break;
				case 6:
					max_subcat = time;
					subcat = std::vector<int>{ 2, 3, 1, 4 };
					break;
				default:
					throw std::invalid_argument("you should have never come here...");
				}

				// check that no errors have been done
				if (max_subcat > subcat.size())
					throw std::invalid_argument("an unexpected error has occurred");
				subcat.resize(max_subcat);
				std::sort(subcat.begin(), subcat.end());

				// finally, iterate over subcategories
				for (int j = 0; j < max_subcat; ++j)
				{
					// apply filters to subcategory
					QList<Instrument*> filteredInstruments = categoriesList->at(i)->applyFiltersToSubcategory(subcat.at(j), istargeted, type, design, approach);

					// create nodes for filtered instruments
					QList<InstrumentNode*> nodes;
					for (int k = 0; k < filteredInstruments.size(); ++k)
						nodes.append(new InstrumentNode(filteredInstruments.at(k)));

					for (int k = 0; k < lastNodes.size(); ++k)
						for (int ii = 0; ii < nodes.size(); ++ii)
							lastNodes.at(k)->appendChild(nodes.at(ii), (nodes.at(ii)->object()->*method)());

					// replace last instruments with new last instrument
					lastNodes = nodes;
				}
			}
			else // category without subcategory
			{
				// apply filters
				QList<Instrument*> filteredInstruments = categoriesList->at(i)->applyFilters(istargeted, type, design, approach);

				// create nodes for filtered instruments
				QList<InstrumentNode*> nodes;
				for (int j = 0; j < filteredInstruments.size(); ++j)
					nodes.append(new InstrumentNode(filteredInstruments.at(j)));
				
				for (int j = 0; j < lastNodes.size(); ++j)
					for (int k = 0; k < nodes.size(); ++k)
						lastNodes.at(j)->appendChild(nodes.at(k), (nodes.at(k)->object()->*method)());

				// replace last instruments with new last instrument
				lastNodes = nodes;
			}
		}

		return tree;
	}

	QString methodsToString(const std::list<std::vector<InstrumentNode*>>& paths)
	{
		QString s;
		int i = 1;
		for (auto it = paths.begin(); it != paths.end(); ++it)
		{
			s += QString::number(i++) + ") ";
			for (int j = 0; j < it->size(); ++j)
			{
				if ((*it)[j]->object() == nullptr)
				{
					s += "START";
				}
				else
				{
					int categoryNum = (*it)[j]->object()->category();
					s += QString::number(categoryNum);
					if (categoryNum == 2 || categoryNum == 4 || categoryNum == 5 || categoryNum == 6 || categoryNum == 7)
						s += "." + QString::number((*it)[j]->object()->subcategory());
					s += " " + (*it)[j]->object()->name();
				}
				s += " -> ";
			}
			s.remove(s.size() - 4, 4);
			s += "\r\n";
		}
		return s;
	}

	QString printInstrumentsGraph(InstrumentsTree& graph)
	{
		// perform DFS
		std::list<std::vector<InstrumentNode*>> paths;
		graph.depthFirstSearch(paths);

		return methodsToString(paths);
	}

	#pragma endregion Static Functions
}