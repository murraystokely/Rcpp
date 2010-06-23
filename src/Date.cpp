// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; tab-width: 8 -*-
//
// Date.h: Rcpp R/C++ interface class library -- Date type
//
// Copyright (C) 2010	      Dirk Eddelbuettel and Romain Francois
//
// The mktime00() function is
// Copyright (C) 2000 - 2010  The R Development Core Team.
//
// This file is part of Rcpp.
//
// Rcpp is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Rcpp is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Rcpp.  If not, see <http://www.gnu.org/licenses/>.

#include <Rcpp/Date.h>
#include <Rcpp/Function.h>

namespace Rcpp {

    const int Date::QLtoJan1970Offset = 25569;  // Offset between R / Unix epoch date and the QL base date

    Date::Date() {
	d = 0; 
    };

    Date::Date(const int &dt) {
	d = dt;
    }

    Date::Date(const std::string &s, const std::string &fmt) {
	Rcpp::Function strptime("strptime");	// we cheat and call strptime() from R
	d = Rcpp::as<int>(strptime(s, fmt));
    }

    Date::Date(const unsigned int &mon, const unsigned int &day, const unsigned int &year) {

	struct tm tm;;
	tm.tm_sec = tm.tm_min = tm.tm_hour = tm.tm_isdst = 0;

	// allow for ISO-notation case (yyyy, mm, dd) which we prefer over (mm, dd, year)
	if (mon >= 1900 && day <= 12 && year <= 31) {
	    tm.tm_year = mon - 1900;
	    tm.tm_mon  = day - 1;       // range 0 to 11
	    tm.tm_mday = year;
	} else {
	    tm.tm_mday  = day;
	    tm.tm_mon   = mon - 1;	// range 0 to 11
	    tm.tm_year  = year - 1900;
	}
	double tmp = mktime00(tm); 	// use mktime() replacement borrowed from R
	d = tmp/(24*60*60);
    }

    Date::Date(const Date &copy) {
	d = copy.d;
    }

    Date & Date::operator=(const Date & newdate) {
	if (this != &newdate) {
	    d = newdate.d;
	}
	return *this;
    }

    // Taken from R's src/main/datetime.c and made a member function called with C++ reference
    /* Substitute for mktime -- no checking, always in GMT */
    double Date::mktime00(struct tm &tm) const {

	static const int days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

        #define isleap(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)
        #define days_in_year(year) (isleap(year) ? 366 : 365)

	int day = 0;
	int i, year, year0;
	double excess = 0.0;

	day = tm.tm_mday - 1;
	year0 = 1900 + tm.tm_year;
	/* safety check for unbounded loops */
	if (year0 > 3000) {
	    excess = (int)(year0/2000) - 1;
	    year0 -= excess * 2000;
	} else if (year0 < 0) {
	    excess = -1 - (int)(-year0/2000);
	    year0 -= excess * 2000;
	}

	for(i = 0; i < tm.tm_mon; i++) day += days_in_month[i];
	if (tm.tm_mon > 1 && isleap(year0)) day++;
	tm.tm_yday = day;

	if (year0 > 1970) {
	    for (year = 1970; year < year0; year++)
		day += days_in_year(year);
	} else if (year0 < 1970) {
	    for (year = 1969; year >= year0; year--)
		day -= days_in_year(year);
	}

	/* weekday: Epoch day was a Thursday */
	if ((tm.tm_wday = (day + 4) % 7) < 0) tm.tm_wday += 7;
	
	return tm.tm_sec + (tm.tm_min * 60) + (tm.tm_hour * 3600)
	    + (day + excess * 730485) * 86400.0;
    }

    Date operator+(const Date &date, int offset) {
	Date newdate(date.d);
	newdate.d += offset;
	return newdate;
    }

    int   operator-(const Date& d1, const Date& d2) { return d2.d - d1.d; }
    bool  operator<(const Date &d1, const Date& d2) { return d1.d < d2.d; }
    bool  operator>(const Date &d1, const Date& d2) { return d1.d > d2.d; }
    bool  operator==(const Date &d1, const Date& d2) { return d1.d == d2.d; };
    bool  operator>=(const Date &d1, const Date& d2) { return d1.d >= d2.d; };
    bool  operator<=(const Date &d1, const Date& d2) { return d1.d <= d2.d; };
    bool  operator!=(const Date &d1, const Date& d2) { return d1.d != d2.d; };

    template <> SEXP wrap(const Date &date) {
	SEXP value = PROTECT(Rf_allocVector(REALSXP, 1));
	REAL(value)[0] = date.getDate();
	Rf_setAttrib(value, R_ClassSymbol, Rf_mkString("Date") ); 
	UNPROTECT(1);
	return value;
    }

}
