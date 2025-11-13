

#include <string>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <algorithm>
#include <random>
#include <unordered_map>
#include "sched.h"

class RR : public Scheduler{
    private:
        //현재 스케줄러의 변하지 않는 총 TQ
        int time_slice_;
        //TQ카운팅용
        int left_slice_;
        //현재 RR방식으로 스케쥴 해야하는 대상들이 모인 큐(즉 레디큐)
        std::queue<Job> waiting_queue;

    public:
        RR(std::queue<Job> jobs, double switch_overhead, int time_slice) : Scheduler(jobs, switch_overhead) {
            name = "RR_"+std::to_string(time_slice);
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능 (아래 코드 수정 및 삭제 가능)
            */
            time_slice_ = time_slice; 
            left_slice_ = time_slice;
        }
        // " 스케줄러는 run()함수가 호출될 때마다, 다음 1초간 실행할 작업 명을 반환한다"
        //즉 run자체는 스케쥴 실행이 아니라, 다음 1초동안 스케쥴해서 돌릴 작업 명을 돌려주는 놈
        //=> 따라서, 조건값 증감은, 반드시 return 직전에 수행해야 함.
        // 즉 run()은, 이전 잡의, "1초 실행 직후의 상황"이라고 보면 된다.

        int run() override {


            //할당된 작업이 없고, => current_job_.name == 0
            // job queue가 비어있지 않은 경우 작업 시작
            //즉 맨처음 시작임
            if(current_job_.name == 0 && !job_queue_.empty()){
                current_job_ = job_queue_.front(); // 현재 작업을맨 앞작업으로 할당
                job_queue_.pop(); // 잡큐에서 팝으로 제거
            }
            //TQ단일 단위(1) 실행 완료시 항상, 새로이 입력된 작업이 존재하는지 확인
            if(!job_queue_.empty())
            {
                if(job_queue_.front().arrival_time <= current_time_){
                    waiting_queue.push(job_queue_.front());
                    job_queue_.pop();
                }
            }
            //작업 종료 검사,(작업이 TQ중간에 종료 되는지도 포함)
            if(current_job_.remain_time == 0){ //현재 작업이 끝난 경우임임
                //완수 시간 저장
                current_job_.completion_time = current_time_;
                //완수 잡 벡터 저장
                end_jobs_.push_back(current_job_);
                //큐 검사
                //동시에 전부 빈경우
                if(waiting_queue.empty() && job_queue_.empty()){
                    return -1;
                }

                //현재 작업 변경 및 대기큐 팝, 시간변경
                current_job_ = waiting_queue.front();
                waiting_queue.pop();
                current_time_ = current_time_ + switch_time_;
                //시간 변경에 따른 검사
                if(!job_queue_.empty())
                {
                    if(job_queue_.front().arrival_time <= current_time_){
                        waiting_queue.push(job_queue_.front());
                        job_queue_.pop();
                    }
                }
                //TQ카운터 초기화
                left_slice_ = time_slice_;

            }else if(left_slice_ == 0){
                //할당된 TQ전부사용
                //TQ전부 사용과, 작업 완전 종료는, 둘중 한놈만 검사하면 된다.
                //=> Q: 잡 완료됨? 
                //  A1: ㅇㅇ => 어 그럼 교체해 
                //  A2: ㄴㄴ => 어그럼 TQ는 전부씀?(지금 여기)
                //의미는 위에서 안걸러진
                //즉 아직 실행시간이 남아있다는 뜻임
                //현재 대기 큐가 비어있는지 확인
                if(waiting_queue.empty()){
                    //TQ코인 충전
                    left_slice_ = time_slice_;
                    //이후 잡 교체 없음 => 문맥교환 x
                }else{;
                    //현재 잡을 대기 큐에 삽입
                    //새로운 잡을 먼저 삽입해줄 필요없음 
                    //=> 위에서 이미 기본단위(1)이 끝난 순간, 새로운 입력이 있는지 검사했음
                    waiting_queue.push(current_job_);
                    //현재 작업 교체
                    current_job_ = waiting_queue.front();
                    //대기큐 팝(즉 ready state => run state로 돌입)
                    waiting_queue.pop();
                    //context swtich time 추가
                    current_time_ = current_time_ + switch_time_;
                    //시간 변경에 따른 검사
                    if(!job_queue_.empty())
                    {
                        if(job_queue_.front().arrival_time <= current_time_){
                            waiting_queue.push(job_queue_.front());
                            job_queue_.pop();
                        }
                    }
                    //TQ코인 충전
                    left_slice_ = time_slice_;
                }
            }

            //첫 실행 업데이트
            if(current_job_.service_time == current_job_.remain_time){

                current_job_.first_run_time = current_time_;
            }    
            //위에서 쓴 조건에 따라서, 이거는 이후 1초동안 실행된 결과를 미리 연산하는 것

            //글로벌타임 ++ => 다음 1초의 작업 이후 글로벌타임을 의미
            current_time_++;
            //현재 작업 남은시간 -- => 다음 1초의 작업 이후 잔여 시간을 의미
            current_job_.remain_time--;
            //TQ 조건 갱신 => 다음 1초의 작업 이후, TQ코인 개수 의미
            left_slice_--;
            return current_job_.name;
        }
                
};

class FeedBack : public Scheduler {
    private:
        std::queue<Job> queue[4]; 
        // 각 요소가 하나의 큐인 배열 선언
        //즉 위의 요놈은, 레디큐임
        int quantum[4] = {1, 1, 1, 1};
        //위의 요놈은 각 레벨의 큐별 TQ를 상징
        int left_slice_;
        //TQ카운터
        int current_queue;
        //재 스케쥴시, 처리를 위한 임시 큐 레벨
        int queue_level_for_re_sched;
    public:
        FeedBack(std::queue<Job> jobs, double switch_overhead, bool is_2i) : Scheduler(jobs, switch_overhead) {
            if (is_2i) {
                name = "FeedBack_2i";
            } else {
                name = "FeedBack_1";
            }
            /*
            * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
            * 나머지는 자유롭게 수정 및 작성 가능
            */
            // Queue별 time quantum 설정
            if (name == "FeedBack_2i") {
                quantum[0] = 1;
                quantum[1] = 2;
                quantum[2] = 4;
                quantum[3] = 8;
            }
            
            /*
            생성자 해석
            => is_2i에 걸리면, 아마 레디큐의 TQ가 2의 제곱으로 커지고
            아니면 그냥 RR마냥 정해진 레벨로 돌아가는듯?(아마도 초기화에 의하면 1)
            */
        }
        // 비어있지 않은, 가장 높은 우선순위의 레디큐 레벨 반환
        // 만약 전부 비어있다면 -1 반환
        int returnHighestQueueLevel(){
            for(int i =0; i <4; i++){
                if(!queue[i].empty()){
                    return i;
                }
            }
            return -1;
        }
        int run() override {
                
            //할당된 작업이 없고, => current_job_.name == 0
            // job queue가 비어있지 않은 경우 작업 시작
            //즉 맨처음 시작임
            if(current_job_.name == 0 && !job_queue_.empty()){
                current_job_ = job_queue_.front(); // 현재 작업을맨 앞작업으로 할당
                job_queue_.pop(); // 잡큐에서 팝으로 제거
                // 첫 시작이므로 레벨 설정 필요, level zero queue
                current_queue = 0;
                //TQ를 현재 레벨의 큐에 맞게 설정
                left_slice_ = quantum[current_queue];
            }
            
            //단일 단위 작업 실행 이후, 항상 새로이 입력된 작업이 존재한느지 확인
            if(!job_queue_.empty()){
                if(job_queue_.front().arrival_time <= current_time_){
                    queue[0].push(job_queue_.front());
                    job_queue_.pop();                        
                }
            }

            if(current_job_.remain_time == 0){
                //현재 작업이, 완료된 경우
                //완료시각 기록
                current_job_.completion_time = current_time_;
                //완료 큐 업데이트
                end_jobs_.push_back(current_job_);
                //모든 스케쥴 종료 검사
                if(returnHighestQueueLevel() == -1 && job_queue_.empty()){
                    return -1;
                }
                //재스케쥴 필요
                //대기중 요소가 존재하는 가장 높은 레디큐 레벨 획득
                queue_level_for_re_sched = returnHighestQueueLevel();

                //레디큐가 전부 비어있을 경우
                if(queue_level_for_re_sched == -1){
                    // printf("\nempty queue error\n");
                    //말이 안되는 조건, 따라서 오류임
                    return -1;
                }               
    
                //현재 상태는 막 작업이 완료된 상태임

                //현재 작업 교체, 가장 최상위 대기큐의 잡을 배치
                current_job_ = queue[queue_level_for_re_sched].front();
                //방금 뽑은 잡, 해당 큐에서 제거
                queue[queue_level_for_re_sched].pop();
                //문맥교환 context swtich 시간 연산
                current_time_ = current_time_ + switch_time_;
                //현재 방금 뽑은 큐의 레벨을 기억
                current_queue = queue_level_for_re_sched;
                //방금 뽑은 큐의 레벨에 맞는, TQ코인 할당
                left_slice_ =  quantum[current_queue];   
                //교환에 따른 새로운 작업 검사
                if(!job_queue_.empty()){
                    if(job_queue_.front().arrival_time <= current_time_){
                        queue[0].push(job_queue_.front());
                        job_queue_.pop();                        
                    }
                }
            } else if(left_slice_ == 0){
                
                //현재 할당받은 TQ를 모두 소진시
                //즉, 방금전 작업이 완료된것이 아님
                /////////////큐변환

                if(!job_queue_.empty()){
                    if(job_queue_.front().arrival_time <= current_time_){
                        queue[0].push(job_queue_.front());
                        job_queue_.pop();                        
                    }
                }
                //할당할 작업이 있는 큐 레벨 획득
                queue_level_for_re_sched = returnHighestQueueLevel();

                //대기 큐가 비어있는 경우
                if(queue_level_for_re_sched == -1 ){ 
                    //이게 아래의 큐 하향조정 작업보다 항상 우선되어야함
                    //고대로 다시 해당 레벨에 맞는 TQ할당
                    left_slice_ = quantum[current_queue];
                }else if(queue_level_for_re_sched > current_queue + 1){ 
                    ///지금 작업에 대한 큐를 하향조정해도, 현재 대기큐에 있는 가장 최상-우선순위 작업보다 우선되어야함
                    //즉 그냥 가상으로 큐만 내리고, 따로 입출력 xx
                    //그냥 다시 스케쥴
                    //현재 작업에 대한 큐를 한칸 아래로 조정
                    //즉 레디큐에 넣었다 빼는 과정만 제거하고, 마치 그렇게 동작한것처럼 설정
                    current_queue = current_queue + 1;
                    //해당 레디큐 레벨에 맞는 TQ설정
                    left_slice_ = quantum[current_queue];
                }else{
                    //아무 조건도 필요없고, 현재 작업을 큐레벨 하향하고, 새로운 최상위 작업을 할당받음
                    //현재 작업 하향조정
                    if(current_queue == 3){
                        //큐 레벨은 최대 3(4단계)까지이므로 넘어가지 않게 처리
                        queue[3].push(current_job_);
                    }else{
                        // 아무 조건 필요없이, 현재 동작했던 잡의 큐레벨을 하나 하향
                        queue[current_queue + 1].push(current_job_);
                    }               
                    //새 최고-우선순위 잡 할당
                    current_job_ = queue[queue_level_for_re_sched].front();
                    //방금 할당한 잡, 큐에서 제거
                    queue[queue_level_for_re_sched].pop();
                    //교환시간추가
                    current_time_ = current_time_ + switch_time_;
                    //현재 큐 레벨 갱신
                    current_queue = queue_level_for_re_sched;
                    //현재 큐 레벨에 맞는 TQ 할당
                    left_slice_ = quantum[queue_level_for_re_sched];
                    //시간흐름에 따른 새 잡 검색
                    if(!job_queue_.empty()){
                        if(job_queue_.front().arrival_time <= current_time_){
                            queue[0].push(job_queue_.front());
                            job_queue_.pop();                        
                        }
                    }
        
                }      
            }
            //TQ가 남아있다면, 즉 공통사항
            //교체되었던 아니던 무조건 실행되어야하는 부분분

            //첫실행 정보 갱신
            if(current_job_.remain_time == current_job_.service_time){
                current_job_.first_run_time = current_time_;
            }
            //글로벌 타임 증가
            current_time_++;
            //현재 잡의 남은시간 감소
            current_job_.remain_time--;
            //현재 잡의 TQ를 감소
            left_slice_--;
            return current_job_.name;
        }
};

class Lottery : public Scheduler{
    private:
        int counter = 0;
        int total_tickets = 0;
        int winner = 0;
        std::mt19937 gen;  // 난수 생성기


        //아래 셋은 구현상 필요
        //job list직접 참조를 위한 잡-구조체형 포인터 변수
        Job* jobptr = NULL;
        //job list에서 추출 또는 뭐시깽이든 할때 필요한 임시 job name
        int target_job_name = 0;
        //가장 최근 실행된 job name 기억용
        int recent_job_name = 0;
        //최대 티켓값
        int max_ticket = 0;
        //최소 티켓값(고정)
        const int min_ticket = 1;
        
    public:
        Lottery(std::list<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "Lottery";
            // 난수 생성기 초기화
            uint seed = 10; // seed 값 수정 금지
            gen = std::mt19937(seed);
            total_tickets = 0;
            for (const auto& job : job_list_) {
                total_tickets += job.tickets;
            }
        }
        //아마도 대충 min max 포함하는 범위에서 무작위변수 리턴하는 함수로 추정
        //사용시 1부터 티켓값까지로 사용(양끝단 포함)
        int getRandomNumber(int min, int max) {
            std::uniform_int_distribution<int> dist(min, max);
            return dist(gen);
        }
        //최대 티켓 개수 갱신 함수
        void reCalTotalTicket(){
            total_tickets = 0;
            //각 joblist를 순회하면서, 모든 티켓값을 갱신
            for(const auto &job : job_list_){
                total_tickets += job.tickets;
            }
            //사용할 변수도 다시 갱신
            max_ticket = total_tickets;
        }

        //RV에 각 잡의 티켓값을 빼가면서, 0포함 음수가 나오면 해당 구간인것으로판단.
        int returnTargetJobName(){
            //하기전, 최대티켓 갱신
            reCalTotalTicket();
            //최소는 항상 1임; 랜덤값 추출
            int random_value = getRandomNumber(min_ticket, max_ticket);
            //리턴값 설정, 목표 잡 이름
            int target_job_name = -1;
            for(auto &job : job_list_){
                //job List를 순회하면서, 순차적으로(순서대로 순회함(잡0부터 끝까지<맨 처음 기준>))
                //무작위 난수에 대한 연산
                random_value = random_value - job.tickets;
                //만일 조건을 충족하면
                if(random_value <= 0){
                    //반환 값 조정
                    target_job_name = job.name;
                    return target_job_name;
                }
            }
            //반환 , 해당 경우는 문제가 생긴 경우
            return target_job_name;
        }
        //인자로 주어진 잡-이름을 바탕으로, job list에 접근하여, 동일한 이름을 가진 객체의 주소(*)를 반환
        Job* returnJobPointer(int input_job_name){
            //반환용 포인터
            Job* jp = NULL;
            //job list를 순회
            for(auto &job: job_list_){
                //만일 내가 찾는 이름과 같은 이름을 가진 잡이 joblist에 존재시
                if(job.name == input_job_name){
                    //조건에 맞는 job 주소 저장
                    jp = &job;
                    //해당 주소 반환
                    return jp;
                }
            }
            //찾지 못한 경우(문제발생)
            return jp;
        }

        int run() override {
            //현재 작업 이름이 0 => 첫실행
            if(current_job_.name == 0){
                //실행해야할 작업의 이름을 획득
                target_job_name = returnTargetJobName();
                //해당 작업과 일치하는, joblist의 작업 정보 획득
                jobptr = returnJobPointer(target_job_name);
                //예외처리
                if(jobptr == NULL){
                    return -1;
                }
                //현재 작업을, 위에서 얻은 작업 정보로 **(값)복사
                current_job_ = *jobptr;
                //최근 실행 작업이름을 갱신
                recent_job_name = current_job_.name;
                //현재 작업의 잔여시간 감소
                current_job_.remain_time--;
                //글로벌 시간 증가
                current_time_++;
                //반환
                //첫 시도이기에 로터리는 이러한 처리가 필요
                return current_job_.name;
            }
            //항상 단위작업을 이행 한 뒤, 현재 작업의 정보를 joblist의 일치하는 요소에 갱신
            *jobptr = current_job_;
            //만일 현재 작업이 완료된경우
            if(current_job_.remain_time == 0){
                //완료시간갱신
                current_job_.completion_time = current_time_;
                //해당 작업 다시 갱신
                *jobptr = current_job_;
                //완료된 작업, 완료벡터에 추가
                end_jobs_.push_back(*jobptr);

                //현재 잡 지우기
                //.earse()함수가, 이터레이터만 인자로 받기 때문에 아래와 같은 형식을 사용
                for(auto remove_target = job_list_.begin(); remove_target != job_list_.end(); ++remove_target){
                    //조건에 맞는(지워야하는 작업) 작업 발견시시
                    if(remove_target -> name == current_job_.name){
                        //job list에서 제거거
                        job_list_.erase(remove_target);
                        break; // ***************매우중요, 지우고 탈출안하면 고장남(지워진 메모리 참조)
                    }
                }  
                //모든 작업 종료 검사
                if(job_list_.empty()){
                    return -1;
                }

                //새작업 할당
                //새작업 이름 획득
                target_job_name = returnTargetJobName();
                //예외처리
                if(target_job_name == -1){
                    return -1;
                }
                //이름에 알맞는 joblist의 작업 정보(주소) 획득
                jobptr =returnJobPointer(target_job_name);
                //예외처리
                if(jobptr == NULL){
                    return -1;
                }
                //현재 작업 정보 갱신
                current_job_ = *jobptr;
                //글로벌 시간 갱신
                current_time_ = current_time_ + switch_time_;
            }else{
                //방금 스케쥴링으로 인해, 작업이 완료된것이 아님
                //따라서 그냥 교체만 필요
                //다음 작업 이름 획득
                target_job_name = returnTargetJobName();
                //방금 바로전에 실행되었던 작업과, 현재 선택된 작업이 동일한 경우
                if(recent_job_name == target_job_name){
                    //아무 변경점도 필요 없음
                }else{
                    //다른 작업의 경우
                    //이름을 바탕으로 주소 획득
                    jobptr = returnJobPointer(target_job_name);
                    //예외처리
                    if(jobptr == NULL){
                        return -1;
                    }
                    //현재 작업 갱신
                    current_job_= *jobptr;
                    //글로벌 시간 갱신(문맥교환 시간)
                    current_time_ = current_time_ + switch_time_;
                }
            }
            //첫 실행 정보 갱신
            if(current_job_.remain_time == current_job_.service_time){
                current_job_.first_run_time = current_time_;
            }
            // 현재 작업의 잔여시간 감소
            current_job_.remain_time--;
            //글로벌 시간 증가
            current_time_++;
            //가장 최신에 작동한 작업의 이름을, 현재 작업이름으로 갱신
            recent_job_name = current_job_.name;
            
            return current_job_.name;
        }
};


class Stride : public Scheduler{
    private:
        // 각 작업의 현재 pass 값과 stride 값을 관리하는 맵
        std::unordered_map<int, int> pass_map;  
        std::unordered_map<int, int> stride_map;  
        const int BIG_NUMBER = 10000; // stride 계산을 위한 상수 (보통 큰 수를 사용)


        //작업 교체등에 피요한 작업 이름
        int target_job_name = 0;
        //가장 최근에 작동한 작업 이름
        int recent_job_name = 0;
        //작업 교체 등에 필요한 잡-구조체 포인터 변수
        Job* jobptr = NULL;


    public:
        Stride(std::list<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
            name = "Stride";
                    // job_list_에 있는 각 작업에 대해 stride와 초기 pass 값(0)을 설정
            for (auto &job : job_list_) {
                // stride = BIG_NUMBER / tickets (tickets는 0이 아님을 가정)
                stride_map[job.name] = BIG_NUMBER / job.tickets;
                pass_map[job.name] = 0;
            }
        }
        //현재 pass map을 기준으로, 가장 작은 value를 가진 pass맵의 key값(즉 잡-이름)을 반환
        int returnMinJobName(){
            //정수 최댓값
            int min = 0x7fffffff;
            //반환할 작업 명
            int min_job_name = 0;   
            //pass map을 순회
            for(auto &v : pass_map){
                //이때 unordered_map 이므로, 순서가 없음

                //따라서 원하는 결과, 순차적으로 검색한것과 동일한 효과
                //순차적으로 검색 했을 시, pass값이 동점인 경우, 앞의 작업을 우선해서 처리
                // => 라는 조건을 지키기 위해서 조건연산
                if(v.second < min){
                    //패스값이 최소라 무조건 갱신
                    min = v.second;
                    min_job_name = v.first;
                }else if(v.second == min){
                    //최솟값(패스 최소값)이 동일하면
                    //더 앞선 작업명(앞의 작업)을 우선해서 갱신
                    if(v.first < min_job_name){
                        min_job_name = v.first;
                        min = v.second;
                    }
                }
            }
            //반환
            return min_job_name;
        }
        //인자로 주어진 잡-이름을 바탕으로, job list에 접근하여, 동일한 이름을 가진 객체의 주소(*)를 반환
        Job* returnJobPointer(int input_job_name){
            //반환 포인터 변수
            Job* jp = NULL;
            //잡리스트 순회
            for(auto &job: job_list_){
                //목표로 하는 잡 발견시
                if(job.name == input_job_name){
                    //해당 잡 주소 저장
                    jp = &job;
                    //반환환
                    return jp;
                }
            }
            // 이거 반환되면 뭔가 잘못된것
            return jp;
        }
        

        int run() override {
            if(current_job_.name == 0){
                //stride에, 초기 값이 전부 0으로 초기화 되어있으므로, 1번 잡부터 시작
                if(!job_list_.empty()){
                    //1번 작업 주소 할당
                    jobptr = returnJobPointer(1);
                    if(jobptr == NULL){
                        return -1;
                    }
                }
                //현재 잡 정보 할당
                //값복사임을 명심할 것
                current_job_ = *jobptr;
                //맨 처음 단계를 처리하기 위해서, 최근의 작업을 임의로 현재 잡으로 설정
                recent_job_name = current_job_.name;
            }

            //1초 단위 작업 후, 변경된 잡정보를 토대로, 잡 리스트의 매칭되는 객체를 갱신
            //구현한 방법상, 반드시 처음에 job list객체를 역으로 갱신시켜주어야함
            // std::cout << "AFTER SCHEDUL THE JOB LIST TARGET ELEMENT INFO UPDATE" << std::endl;
            *jobptr = current_job_;
            

            //만일 현재 작업이 완료 된 경우
            if(current_job_.remain_time == 0){
                //현재 작업 완료시간 갱신
                current_job_.completion_time = current_time_;
                //현재 작업을 잡리스트의 매칭되는 작업에 갱신
                *jobptr = current_job_;
                
                //잡 리스트에 접근해, 현재 실행중인 잡에 대한 완료 시간을 갱신
                jobptr -> completion_time = current_time_;

                //end job 벡터: 새로운 완료된 작업 추가
                end_jobs_.push_back(*jobptr);
                


                //다끝낸 잡을 리스트에서 제거, erase()때문에 해당 이터레이터 방식 사용
                for(auto remove_target = job_list_.begin(); remove_target != job_list_.end(); ++remove_target){
                    //제거 잡 발견시
                    if(remove_target -> name == current_job_.name){
                        //제거거
                        job_list_.erase(remove_target);
                        break; // 탈출 *****(중요****)
                    }
                }  
                //다끝낸 잡에 대한 맵 제거, 패스, 스트라이드 둘다
                pass_map.erase(current_job_.name);
                stride_map.erase(current_job_.name);

                //모든 프로그램 완료 검사
                if(job_list_.empty()){
                    return -1;
                }

                //다시 스케쥴링 해야하는 경우

                //가장 작은 pass값을 가진, 잡 이름 획득
                target_job_name = returnMinJobName();
                //잡이름을 바탕으로, job list의 객체에 직접 접근
                jobptr = returnJobPointer(target_job_name);
                //예외처리
                if(jobptr == NULL){
                    return -1;
                }

                //위에서 접근한 객체를 복사하여, 현재 잡으로 할당
                current_job_ = *jobptr;
                //현재 시간을 문맥교환 시간을 더한 정보로 갱신
                current_time_ = current_time_ + switch_time_;
            } else{
                //끝난거 아니면 무조건 재-스케쥴해야함(다시 돌려도 일단 찾아는 봐야함)

                //가장 작은 pass값을 가진, 잡 이름 획득
                target_job_name = returnMinJobName();

                //만일 지금 시도되어야하는 잡이, 직전에도 실행되었던 잡이라면
                if(target_job_name == recent_job_name){
                    //그냥 넘어가기
                    //맨 처음 케이스의 경우에도 여길 통해서 그냥 넘어감
                }else{
                    //시도되어야하는 잡이, 이전 잡과 별개의 잡이라면
                    //잡 이름을 바탕으로, 다시 job list의 매칭 객체에 접근
                    jobptr = returnJobPointer(target_job_name);
                    if(jobptr == NULL){
                        return -1;
                    }
                    //접근한 객체를 복사하여, 현재 잡에 할당
                    current_job_ = *jobptr;
                    //문맥교환시간 추가
                    current_time_ = current_time_ + switch_time_;
                }
            }

            // 첫 스케쥴 일시, 정보 갱신
            if(current_job_.remain_time == current_job_.service_time){
                current_job_.first_run_time = current_time_;
            }


            //글로벌 시간 증가
            current_time_++;
            //현재 잡의 남은시간 감소
            current_job_.remain_time--;
            //현재 잡에 해당하는 패스맵의 값 stride만큼 증가
            pass_map[current_job_.name] = pass_map[current_job_.name] + stride_map[current_job_.name];
            //실행될 잡의 이름을 임시 저장
            recent_job_name = current_job_.name;
            //다음 1초 뒤 스케쥴링될 작업명 반환
            return current_job_.name; 
        }
};
