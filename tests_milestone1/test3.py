class Student:
    def __init__(self, name, id):
        self.name = name
        self.id = id
        self.courses = []

    def enroll_course(self, course):
        if len(course.students) < course.capacity:
            self.courses.append(course)
            course.register_student(self)
            course.capacity -= 1
        else:
            print("Course is full, cannot enroll.")

    def drop_course(self, course):
        if course in self.courses:
            self.courses.remove(course)
            course.unregister_student(self)
            course.capacity += 1
        else:
            print("Student is not enrolled in this course.")

    def view_courses(self):
        print(f"Student {self.name} is enrolled in the following courses:")
        if self.courses:
            for course in self.courses:
                print(f"- {course.title}")
        else:
            print("No courses enrolled.")

    def view_course_students(self, university):
        print(f"Student {self.name} is enrolled in the following courses with their respective students:")
        if self.courses:
            for course in self.courses:
                print(f"- {course.title}:")
                course_students = [student.name for student in course.students]
                if course_students:
                    for student_name in course_students:
                        print(f"  - {student_name}")
                else:
                    print("   No students enrolled.")
        else:
            print("No courses enrolled.")


class Course:
    def __init__(self, title, code, capacity):
        self.title = title
        self.code = code
        self.capacity = capacity
        self.students = []

    def register_student(self, student):
        self.students.append(student)

    def unregister_student(self, student):
        if student in self.students:
            self.students.remove(student)

    def view_students(self):
        print(f"Students enrolled in course {self.title} (Code: {self.code}):")
        if self.students:
            for student in self.students:
                print(f"- {student.name} (ID: {student.id})")
        else:
            print("No students enrolled.")

    def view_course_students(self):
        print(f"Students enrolled in course {self.title} (Code: {self.code}):")
        if self.students:
            for student in self.students:
                print(f"- {student.name} (ID: {student.id})")
        else:
            print("No students enrolled.")


class University:
    def __init__(self, name):
        self.name = name
        self.students = []
        self.courses = []

    def add_student(self, name, id):
        student = Student(name, id)
        self.students.append(student)

    def add_course(self, title, code, capacity):
        course = Course(title, code, capacity)
        self.courses.append(course)

    def view_students(self):
        print(f"Students at {self.name}:")
        if self.students:
            for student in self.students:
                print(f"- {student.name} (ID: {student.id})")
        else:
            print("No students found.")

    def view_courses(self):
        print(f"Courses offered at {self.name}:")
        if self.courses:
            for course in self.courses:
                print(f"- {course.title} (Code: {course.code})")
        else:
            print("No courses available.")

    def enroll_students_in_courses(self):
        for student in self.students:
            for course in self.courses:
                if student.id % 3 == 0 and course.capacity > 0:
                    student.enroll_course(course)

# Example usage
if __name__ == "__main__":
    # Create University
    university = University("ABC University")

    # Add Students
    university.add_student("John Doe", 1001)
    university.add_student("Alice Smith", 1002)
    university.add_student("Bob Johnson", 1003)

    # Add Courses
    university.add_course("Mathematics", "MATH101", 30)
    university.add_course("Physics", "PHYS101", 25)
    university.add_course("Computer Science", "CS101", 20)

    # Enroll Students in Courses
    university.enroll_students_in_courses()

    # View Students and Courses
    university.view_students()
    print()
    university.view_courses()
    print()
    university.courses[0].view_students()
    print()
    university.students[0].view_courses()
    print()
    university.students[0].view_course_students(university)
