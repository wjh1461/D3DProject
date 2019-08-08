#pragma once
class GameTimer
{
public:
	GameTimer();

	float TotalTime() const;
	float DeltaTime() const;

	// 시작할 때 호출
	void Start();
	// 매 프레임마다 호출
	void Tick();
	// 메시지 루프 전에 호출
	void Reset();
	// 멈출 때 호출
	void Stop();

private:
	double mSecondsPerCount;
	double mDeltaTime;

	__int64 mBaseTime;
	// 정지된 시간
	__int64 mPausedTime;
	// 정지 시점의 시간
	__int64 mStopTime;
	// 이전 프레임의 시간
	__int64 mPrevTime;
	// 현재 프레임의 시간
	__int64 mCurrTime;

	// 정지 여부
	bool mStopped;
};

