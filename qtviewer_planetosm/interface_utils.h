#pragma once
#include <QVariant>
#include <QMap>
#include <QString>
#include <QVariant>
#define QTVOSM_DEBUG(MSG) qDebug()<<"QTVOSM Debug>"<< MSG <<"\n\t"<<__FUNCTION__<<":"<<__FILE__<<__LINE__
#define QTVOSM_WARNING(MSG) qWarning()<<"QTVOSM Debug>"<< MSG <<"\n\t"<<__FUNCTION__<<":"<<__FILE__<<__LINE__
#define QTVOSM_FATAL(MSG) qFatal()<<"QTVOSM Debug>"<< MSG <<"\n\t"<<__FUNCTION__<<":"<<__FILE__<<__LINE__
/**
 * @brief map_to_string Convert QMap key-value paires to string
 * @param m map. m["A"]=12, m["B"] = "Yes"
 * @return  string , eg: "A=12;B=Yes;"
 */
inline QString map_to_string(const QMap<QString, QVariant> & m)
{
	QString s;
	for(QMap<QString, QVariant>::const_iterator p = m.begin();p!=m.end();++p)
	{
		s += p.key();
		s += "=";
		s += p.value().toString();
		s += ";";
	}
	return /*std::move*/(s);
}
/*!
 * \brief string_to_map Convert string to key-value map
 * \param s string, eg: "A=12;B=Yes;"
 * \return map. map["A"]=12, map["B"] = "Yes"
 */
inline QMap<QString, QVariant> string_to_map(const QString & s)
{
	QMap<QString, QVariant> res;
	QStringList lst = s.split(";");
	foreach (QString s, lst)
	{
		int t = s.indexOf("=");
		if (t>0 && t< s.size())
		{
			QString name = s.left(t).trimmed();
			QString value = s.mid(t+1).trimmed();
			res[name] = value;
		}
	}
	return /*std::move*/(res);
}
