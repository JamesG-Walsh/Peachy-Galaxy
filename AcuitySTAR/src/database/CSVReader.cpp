#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <QDebug>
#include <QString>

#include "database/CSVReader.h"

// Loads the CSV file at file_name.

void CSVReader::loadCSV(std::string file_name)
{
    std::ifstream myfile(file_name.c_str());

    if (myfile.fail())
    {
        std::cout << "Couldn't open file \"" << file_name << "\"." << std::endl;
        return;
    }

    if (myfile.is_open())
    {
        std::stringstream sstr;
        sstr << myfile.rdbuf();
        std::string f = sstr.str();
        myfile.close();

        size_t len = f.length();

        bool setHeaders = false;

        size_t pos = 0;
        while( pos < len ) { // While the file's not empty
            std::vector<std::string> line;
            while( f.at( pos ) != '\n' && pos < len ) { // For each character in the line
                std::string element = "";
                while( f.at( pos ) != ',' && pos < len && f.at( pos ) != '\n' && f.at( pos ) != '\r' ) { // For each element
                    if( f.at( pos ) == '"' ) { // If we have a quote, continue till the next quote
                        pos++;
                        while( f[pos] != '"' && pos < len ) {
                            element += f.at( pos );
                            pos++;
                        }
                        pos++; // Last quote
                    } else {
                        element += f.at( pos );
                        pos++;
                    }
                }
                line.push_back( element );

                if ( f.at( pos ) == '\n') {
                    break;
                }
                pos++;
            }
            if( !setHeaders ) {
                setHeaders = true;
                headers = line;
            } else {
                all_data.push_back( line );
            }
            pos++;
        }
    }
}

// Returns the year in a date string.

int CSVReader::parseDateString(std::string dateString) {
    int year;
    sscanf(dateString.c_str(), "%4d", &year); // The only 4-digit number in a date field is a year, period //not anymore :(
    return year;
}

// Returns the header vector for a CSV

std::vector<std::string> CSVReader::getHeaders() {
    return headers;
}

// Returns all data from a CSV

std::vector<std::vector<std::string>> CSVReader::getData() {
    return all_data;
}

void CSVReader::fixDateFormatting()
{
    std::vector<std::string> theHeaders = this->getHeaders();
    std::vector<std::vector<std::string>> theData = this->getData();
    int dateIndex;

    for (int i = 0; i < theHeaders.size() ; i++)
    {
        if (theHeaders.at(i) == "Start Date" || theHeaders.at(i) == "Date" || theHeaders.at(i) == "Status Date")
        {
            dateIndex = i;
        }
    }

    //qDebug() << "date index: " << dateIndex;

    std::vector<std::string> currentRow;
    std::string inputString;
    int count = 0;
    for (int i = 0 ; i < theData.size() ; i++)
    {
        currentRow = theData.at(i);
        inputString = currentRow.at(dateIndex);
        int twoDigitYear;
        int resultingFourDigitYear;

        if(inputString.length() == 6)
        {
            count++;

            //std::cout << "\n";
            //std::cout << inputString;

            QString qDateString = QString::fromStdString(inputString);
            //qDebug() << "\n" << qDateString;
            if (qDateString.startsWith("J") || qDateString.startsWith("F") || qDateString.startsWith("M") || qDateString.startsWith("A") || qDateString.startsWith("S") || qDateString.startsWith("O") || qDateString.startsWith("N") || qDateString.startsWith("D"))
            {
                //qDebug() << "year at end\t";
                qDateString.remove(0,4);
            }
            else
            {
                //qDebug() << "year in front\t";
                qDateString.chop(4);

            }
            //qDebug() << qDateString;
            twoDigitYear = qDateString.toInt();
            //qDebug() << twoDigitYear;


            //sscanf(inputString.c_str(), "%2d", &twoDigitYear);

            //std::cout << "\t" << twoDigitYear;

            if( 0 <= twoDigitYear && twoDigitYear < 50)
            {
                resultingFourDigitYear = twoDigitYear + 2000;
            }
            else if (50 <= twoDigitYear && twoDigitYear < 100)
            {
                resultingFourDigitYear = twoDigitYear + 1900;
            }

            //qDebug() << resultingFourDigitYear;

            std::string finalS = std::to_string(resultingFourDigitYear);

            all_data.at(i).at(dateIndex) = finalS; //put new 4 character string into the CSVReader attribute 2D vector.

            //std::cout << "\t" << resultingFourDigitYear;
        }

    }
    //qDebug() << "count: " << count;

}

CSVReader::CSVReader()
{
    //qDebug() << "Paul wins";
    //fixDateFormatting();
}

CSVReader::CSVReader(std::string file) {
    loadCSV(file);
    fixDateFormatting();

    std::vector<std::string> currentRow;
    std::string emptyString = "";

    for(int row = 0; row < all_data.size() ; row++)
    {
        currentRow = all_data.at(row);
        if (currentRow.size() < headers.size())
        {
            qDebug() << "row: " << row << "\tsize: " << currentRow.size();
            all_data.at(row).push_back(emptyString);
        }
    }
    qDebug() << "done?";
}
