#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "../Project20/BookingScheduler.cpp"

using namespace std;

class Sunday : public BookingScheduler {
	time_t getTime(int year, int mon, int day, int hour, int min) {
		tm result = { 0,min,hour,day,mon - 1,year - 1900, 0,0,-1 };
		return mktime(&result);
	}
public:
	Sunday(int cap) :
		BookingScheduler{ cap } {}

	time_t getNow() override {
		return getTime(2021, 3, 28, 17, 0);
	}
};

class Monday : public BookingScheduler {
	time_t getTime(int year, int mon, int day, int hour, int min) {
		tm result = { 0,min,hour,day,mon - 1,year - 1900, 0,0,-1 };
		return mktime(&result);
	}
public:
	Monday(int cap) :
		BookingScheduler{ cap } {}

	time_t getNow() override {
		return getTime(2024, 6, 3, 17, 0);
	}
};


class TestableMailSender : public MailSender {
	int countSendMailMethodIsCalled = 0;
public:
	void sendMail(Schedule* schudule) override {
		countSendMailMethodIsCalled++;
	}
	int getCountSendMailMethodIsCalled() {
		return countSendMailMethodIsCalled;
	}
};

class TestableSmsSender : public SmsSender {
	bool sendMethodIsCalled;
public:
	void send(Schedule* schudule) override {
		cout << "�׽�Ʈ�� SmsSender" << endl;
		sendMethodIsCalled = true;
	}
	bool isSendMethodIsCalled() {
		return sendMethodIsCalled;
	}
};

class BookingItem : public testing::Test {
public:
	tm getTime(int year, int mon, int day, int hour, int min) {
		tm result = { 0,min,hour,day,mon - 1,year - 1900, 0,0,-1 };
		mktime(&result);
		return result;
	}
	void SetUp() override {
		NOT_ON_THE_HOUR = getTime(2021, 3, 26, 9, 5);
		ON_THE_HOUR = getTime(2021, 3, 26, 9, 0);
		bookingScheduler.setSmsSender(&test);
		bookingScheduler.setMailSender(&testMail);
	}
	tm plusHour(tm base, int hour) {
		base.tm_hour += hour;
		mktime(&base);
		return base;
	}

	tm NOT_ON_THE_HOUR;
	tm ON_THE_HOUR;
	Customer CUSTOMER{ "Fake", "010-1234-5678" };
	Customer customerWithMail{ "Fake", "010-1234-1234", "test@test.com" };
	const int UNDER_CAPACITY = 1;
	const int CAPACITY_PER_HOUR = 3;
	BookingScheduler bookingScheduler{ CAPACITY_PER_HOUR };
	TestableSmsSender test;
	TestableMailSender testMail;
};

TEST_F(BookingItem, ������_���ÿ���_�����ϴ�_���ð�_�ƴѰ��_����Ұ�) {
	Schedule* schedule = new Schedule{ NOT_ON_THE_HOUR, UNDER_CAPACITY, CUSTOMER };

	EXPECT_THROW({ bookingScheduler.addSchedule(schedule); }, runtime_error);
}

TEST_F(BookingItem, ������_���ÿ���_�����ϴ�_������_���_���డ��) {
	Schedule* schedule = new Schedule{ ON_THE_HOUR, UNDER_CAPACITY, CUSTOMER };
	bookingScheduler.addSchedule(schedule);

	EXPECT_EQ(true, bookingScheduler.hasSchedule(schedule));
}

TEST_F(BookingItem, �ð��뺰_�ο�������_�ִ�_����_�ð��뿡_Capacity_�ʰ���_���_���ܹ߻�) {
	Schedule* schedule = new Schedule{ ON_THE_HOUR, CAPACITY_PER_HOUR, CUSTOMER };
	bookingScheduler.addSchedule(schedule);

	try {
		Schedule* newSchedule = new Schedule{ ON_THE_HOUR,UNDER_CAPACITY,CUSTOMER };
		bookingScheduler.addSchedule(newSchedule);
		FAIL();
	}
	catch (runtime_error& e) {
		EXPECT_EQ(string{ e.what() }, string{ "Number of people is over restaurant capacity per hour" });
	}
}

TEST_F(BookingItem, �ð��뺰_�ο�������_�ִ�_����_�ð��밡_�ٸ���_Capacity_���־_������_�߰�_����) {
	Schedule* schedule = new Schedule{ ON_THE_HOUR,CAPACITY_PER_HOUR,CUSTOMER };
	bookingScheduler.addSchedule(schedule);

	tm differentHour = plusHour(ON_THE_HOUR, 1);
	Schedule* newSchdule = new Schedule{ differentHour, UNDER_CAPACITY,CUSTOMER };
	bookingScheduler.addSchedule(newSchdule);

	EXPECT_EQ(true, bookingScheduler.hasSchedule(schedule));
}

TEST_F(BookingItem, ����Ϸ��_SMS��_������_�߼�) {
	Schedule* schedule = new Schedule(ON_THE_HOUR, CAPACITY_PER_HOUR, CUSTOMER);

	bookingScheduler.addSchedule(schedule);

	EXPECT_EQ(true, test.isSendMethodIsCalled());
}

TEST_F(BookingItem, �̸�����_����_��쿡��_�̸���_�̹߼�) {
	Schedule* schedule = new Schedule(ON_THE_HOUR, CAPACITY_PER_HOUR, CUSTOMER);

	bookingScheduler.addSchedule(schedule);

	EXPECT_EQ(0, testMail.getCountSendMailMethodIsCalled());
}

TEST_F(BookingItem, �̸�����_�ִ�_��쿡��_�̸���_�߼�) {
	Schedule* schedule = new Schedule(ON_THE_HOUR, CAPACITY_PER_HOUR, customerWithMail);

	bookingScheduler.addSchedule(schedule);

	EXPECT_EQ(1, testMail.getCountSendMailMethodIsCalled());
}

TEST_F(BookingItem, ���糯¥��_�Ͽ�����_���_����Ұ�_����ó��) {
	BookingScheduler* bs = new Sunday(CAPACITY_PER_HOUR);

	try {
		Schedule* schedule = new Schedule{ ON_THE_HOUR, UNDER_CAPACITY, customerWithMail };
		bs->addSchedule(schedule);
	}
	catch (runtime_error& e) {
		EXPECT_EQ(string{ e.what() }, string{ "Booking system is not available on sunday" }); \
	}
}

TEST_F(BookingItem, ���糯¥��_�Ͽ�����_�ƴѰ��_���డ��) {
	BookingScheduler* bs = new Monday(CAPACITY_PER_HOUR);


	Schedule* schedule = new Schedule{ ON_THE_HOUR, UNDER_CAPACITY, customerWithMail };
	bs->addSchedule(schedule);

	EXPECT_EQ(true, bs->hasSchedule(schedule));
}