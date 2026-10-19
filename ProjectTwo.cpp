// CS 300 Project Two - ABCU Advising Assistance Program
// Uses a hash table for course lookup and sorts course numbers for display.
// All functions are contained in this one C++ source file.

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// Stores a course and the course numbers of its prerequisites.
struct Course {
    string courseNumber;
    string courseTitle;
    vector<string> prerequisites;
};

// Remove whitespace and Windows carriage returns from either end of a field.
string trim(const string& value) {
    const size_t first = value.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        return "";
    }
    const size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

// Normalize course numbers so a lowercase search (csci400) works.
string uppercase(string value) {
    transform(value.begin(), value.end(), value.begin(),
        [](unsigned char ch) { return static_cast<char>(toupper(ch)); });
    return value;
}

// Read a CSV line into fields; empty trailing fields are harmless.
vector<string> splitCSV(const string& line) {
    vector<string> fields;
    string field;
    stringstream stream(line);
    while (getline(stream, field, ',')) {
        fields.push_back(trim(field));
    }
    return fields;
}

// Load courses into a temporary hash table so a failed load does not
// overwrite the previously loaded data.
bool loadCourses(const string& fileName,
    unordered_map<string, Course>& courses) {
    ifstream input(fileName);
    if (!input.is_open()) {
        cout << "Error: Unable to open file: " << fileName << endl;
        return false;
    }

    unordered_map<string, Course> newCourses;
    string line;
    size_t lineNumber = 0;

    while (getline(input, line)) {
        ++lineNumber;
        if (trim(line).empty()) {
            continue;
        }

        const vector<string> fields = splitCSV(line);
        if (fields.size() < 2 || fields[0].empty() || fields[1].empty()) {
            cout << "Error: Invalid course data on line " << lineNumber << "." << endl;
            return false;
        }

        Course course;
        course.courseNumber = uppercase(fields[0]);
        course.courseTitle = fields[1];
        for (size_t i = 2; i < fields.size(); ++i) {
            if (!fields[i].empty()) {
                course.prerequisites.push_back(uppercase(fields[i]));
            }
        }

        if (newCourses.find(course.courseNumber) != newCourses.end()) {
            cout << "Error: Duplicate course number on line " << lineNumber << "." << endl;
            return false;
        }
        newCourses.emplace(course.courseNumber, course);
    }

    if (newCourses.empty()) {
        cout << "Error: The file contains no course records." << endl;
        return false;
    }

    // Ensure every prerequisite points to a valid course.
    for (const auto& entry : newCourses) {
        for (const string& prerequisite : entry.second.prerequisites) {
            if (newCourses.find(prerequisite) == newCourses.end()) {
                cout << "Error: " << entry.first << " has an unknown prerequisite: "
                    << prerequisite << endl;
                return false;
            }
        }
    }

    courses.swap(newCourses);
    cout << "Successfully loaded " << courses.size() << " courses." << endl;
    return true;
}

// Sort the hash table's keys to display courses alphanumerically.
void printCourseList(const unordered_map<string, Course>& courses) {
    vector<string> courseNumbers;
    courseNumbers.reserve(courses.size());
    for (const auto& entry : courses) {
        courseNumbers.push_back(entry.first);
    }
    sort(courseNumbers.begin(), courseNumbers.end());

    cout << "\nHere is a sample schedule:\n\n";
    for (const string& number : courseNumbers) {
        const Course& course = courses.at(number);
        cout << course.courseNumber << ", " << course.courseTitle << endl;
    }
}

// Display the selected course and full prerequisite names.
void printCourse(const unordered_map<string, Course>& courses,
    const string& requestedNumber) {
    const string number = uppercase(trim(requestedNumber));
    const auto found = courses.find(number);
    if (found == courses.end()) {
        cout << "Error: Course " << number << " was not found." << endl;
        return;
    }

    const Course& course = found->second;
    cout << "\n" << course.courseNumber << ", " << course.courseTitle << endl;
    if (course.prerequisites.empty()) {
        cout << "Prerequisites: None" << endl;
        return;
    }

    cout << "Prerequisites:" << endl;
    for (const string& prerequisite : course.prerequisites) {
        const Course& requiredCourse = courses.at(prerequisite);
        cout << "  " << requiredCourse.courseNumber << ", "
            << requiredCourse.courseTitle << endl;
    }
}

void printMenu() {
    cout << "\n1. Load Data Structure." << endl;
    cout << "2. Print Course List." << endl;
    cout << "3. Print Course." << endl;
    cout << "9. Exit" << endl;
    cout << "\nWhat would you like to do? ";
}

int main() {
    unordered_map<string, Course> courses;  // Hash table for fast lookup.
    string option;

    cout << "Welcome to the course planner." << endl;
    while (true) {
        printMenu();
        if (!getline(cin, option)) {
            cout << "\nThank you for using the course planner!" << endl;
            break;
        }
        option = trim(option);

        if (option == "1") {
            string fileName;
            cout << "Enter the course data filename: ";
            if (!getline(cin, fileName)) {
                cout << "\nInput ended." << endl;
                break;
            }
            fileName = trim(fileName);
            if (fileName.empty()) {
                cout << "Error: Please enter a filename." << endl;
            }
            else {
                loadCourses(fileName, courses);
            }
        }
        else if (option == "2") {
            if (courses.empty()) {
                cout << "Please load the course data first using option 1." << endl;
            }
            else {
                printCourseList(courses);
            }
        }
        else if (option == "3") {
            if (courses.empty()) {
                cout << "Please load the course data first using option 1." << endl;
            }
            else {
                string courseNumber;
                cout << "What course do you want to know about? ";
                if (!getline(cin, courseNumber)) {
                    cout << "\nInput ended." << endl;
                    break;
                }
                printCourse(courses, courseNumber);
            }
        }
        else if (option == "9") {
            cout << "Thank you for using the course planner!" << endl;
            break;
        }
        else {
            cout << (option.empty() ? "That" : option)
                << " is not a valid option." << endl;
        }
    }
    return 0;
}
