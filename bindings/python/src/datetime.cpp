// Copyright Daniel Wallin 2006. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "boost_python.hpp"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "optional.hpp"
#include <boost/version.hpp>
#include "libtorrent/time.hpp"
#include <ctime>

using namespace boost::python;
namespace lt = libtorrent;

#if BOOST_VERSION < 103400

// From Boost 1.34
object import(str name)
{
    // should be 'char const *' but older python versions don't use 'const' yet.
    char *n = extract<char *>(name);
    handle<> module(borrowed(PyImport_ImportModule(n)));
    return object(module);
}

#endif

object datetime_timedelta;
object datetime_datetime;

template <typename Duration>
struct chrono_duration_to_python
{
    static PyObject* convert(Duration const& d)
    {
        std::int64_t const us = lt::total_microseconds(d);
        object result = datetime_timedelta(
            0 // days
          , us / 1000000 // seconds
          , us % 1000000 // microseconds
        );

        return incref(result.ptr());
    }
};

struct time_duration_to_python
{
    static PyObject* convert(boost::posix_time::time_duration const& d)
    {
        object result = datetime_timedelta(
            0 // days
          , 0 // seconds
          , d.total_microseconds()
        );

        return incref(result.ptr());
    }
};

struct time_point_to_python
{
    static PyObject* convert(lt::time_point pt)
    {
        using std::chrono::system_clock;
        using std::chrono::duration_cast;
        time_t const tm = system_clock::to_time_t(system_clock::now()
            + duration_cast<system_clock::duration>(pt - lt::clock_type::now()));

        std::tm* date = std::gmtime(&tm);
        object result = datetime_datetime(
            (int)1900 + date->tm_year
          , (int)date->tm_mon
          , (int)date->tm_mday
          , date->tm_hour
          , date->tm_min
          , date->tm_sec
        );
        return incref(result.ptr());
    }
};

struct ptime_to_python
{
    static PyObject* convert(boost::posix_time::ptime const& pt)
    {
        boost::gregorian::date date = pt.date();
        boost::posix_time::time_duration td = pt.time_of_day();

        object result = datetime_datetime(
            (int)date.year()
          , (int)date.month()
          , (int)date.day()
          , td.hours()
          , td.minutes()
          , td.seconds()
        );

        return incref(result.ptr());
    }
};

void bind_datetime()
{
    object datetime = import("datetime").attr("__dict__");

    datetime_timedelta = datetime["timedelta"];
    datetime_datetime = datetime["datetime"];

    to_python_converter<boost::posix_time::time_duration
      , time_duration_to_python>();

    to_python_converter<boost::posix_time::ptime
      , ptime_to_python>();

    to_python_converter<lt::time_point
      , time_point_to_python>();

    to_python_converter<lt::time_duration
      , chrono_duration_to_python<lt::time_duration>>();

    to_python_converter<std::chrono::seconds
      , chrono_duration_to_python<std::chrono::seconds>>();

    optional_to_python<boost::posix_time::ptime>();
}

