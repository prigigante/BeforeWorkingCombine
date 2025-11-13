
#include "skiplist_impl.h"
#include <cstdlib>
#include <ctime>

// DefaultSkipList 생성자
DefaultSkipList::DefaultSkipList(int max_level, float prob) {
    //해당 리스트 최대 레벨 지정
    this->max_level_ = max_level; 
    //확률 빈도수 저장 => 차후 랜덤값 뽑을 떄 사용
    this->prob_ = prob;

    // 헤더 생성, new 로 객체 할당 및 값 초기화화
    header_ = new Node();
    header_->key = -1;
    header_->value = -1;
    header_->upd_cnt = 0;
    header_->level = max_level_;
    //헤더에 노드하나 할당하고, 내용은 키-1 값-1 업데이트카운트0 레벨최대대
    //아마 아래 코드를 보아하니, 지금 할당한 헤더는 ㄹㅇ 찐 헤더의 헤더 그자체인듯
    //이 메모리 할당된 구조체를 헤더로 사용하여, 포인터 배열을 레벨의 스타트포인트로 이용하는듯?

    // 노드에서 가리키는 forward 초기화
    //forward가 각 레벨의 start 포인트
    header_->forward = new Node *[max_level_ + 1];
    for (int i = 0; i <= max_level_; i++) {
        header_->forward[i] = nullptr;
    }

    // 랜덤값 시드 할당, 현재시간으로 할당
    srand(time(nullptr)); 
}

// DefaultSkipList 소멸자
DefaultSkipList::~DefaultSkipList() {
    Node *current = header_; // header 노드 추적
    Node *temp; // 삭제에 필요한 임시 포인터

    while (current) { // temp로 순회하며 노드들 삭제
        temp = current->forward[0];
        delete[] current->forward;
        delete current;
        current = temp; // 노드 이동
    }
}// 주어진 소멸자 사용용

//랜덤 레벨 함수
// 확률 빈도수에 의하여, 값을 상승시킬지 결정
// => 즉, 하위 레벨이 보다 많고, 상위 레벨은 보다 적음음
int DefaultSkipList::random_level() {
    int level = 0;
    while (((double)rand() / RAND_MAX) < prob_ && level < max_level_) {
        level++;
    }
    return level;
}

// SkipList 생성자
SkipList::SkipList(int max_level, float prob)
    : DefaultSkipList(max_level, prob) {}

// SkipList 소멸자
SkipList::~SkipList() {}

void SkipList::insert(int key, int value) {
    // 현재 노드: now_node // 현재 노드의 이전 노드: pre_node
    Node *now_node = header_,  *pre_node[max_level_ + 1];

    //  포인터 초기화 작업업
    for (int loop = 0; loop <= max_level_; loop++){
        pre_node[loop] = nullptr; 
    }
    
    // 입력받은 키를 새로 삽입할 지점 탐색
    // 레벨을 따라 검색하며, 삽입될 조건이 된다면(key가 작음) 결정
    for (int now_level = max_level_; now_level >= 0; now_level--) {   
        while (now_node->forward[now_level] != nullptr 
            && now_node->forward[now_level]->key < key) {
            now_node = now_node->forward[now_level];
        }
        pre_node[now_level] = now_node; 
    }

    now_node = now_node->forward[0]; 
    
    // 해당 키가 이미 존재하는 경우우
    // 조건에 따라 갱신 후 종료
    if (now_node != nullptr && now_node->key == key) {
        now_node->value += value;
        now_node->upd_cnt++;
        return;
    }
    
    // 존재 하지 않는 경우 => 삽입입
    // 노드의 키, 값 등등 초기화
    int new_level = random_level();
    Node *insert_node = new Node();
    insert_node->key = key;
    insert_node->value = value;
    insert_node->upd_cnt = 0;
    insert_node->level = new_level;
    insert_node->forward = new Node *[new_level + 1];

    // 노드 삽입후, 이전과 연결결
    for (int i = 0; i <= new_level; i++) 
    {
        insert_node->forward[i] = pre_node[i]->forward[i];
        pre_node[i]->forward[i] = insert_node;
    }
}

int SkipList::lookup(int key) {
    // 순회 추적 위한 노드
    Node *temp_node = header_; 
    
    // 내려가면서 탐색색
    for (int now_level = max_level_; now_level >= 0; now_level--) {
        while (temp_node->forward[now_level] != nullptr 
            && temp_node->forward[now_level]->key < key) {
            temp_node = temp_node->forward[now_level];
        }
    }
    temp_node = temp_node->forward[0];   
    //조건에 따라서 처리
    //찾으면 그값, 아니면 0
    if (temp_node != nullptr && temp_node->key == key) 
    {
        return temp_node->value;
    }
    return 0;
}

void SkipList::remove(int key) {
    //사용할 지금노드, 이전노드
    Node *now_node = header_ , *pre_node[max_level_ + 1]; 
    // 이전 노드 초기화
    for (int loop = 0; loop <= max_level_; loop++){
        pre_node[loop] = nullptr; 
    }
    // 가상 상위 레벨로부터 하위(0) 까지 탐색
    for (int now_level = max_level_; now_level >= 0; now_level--) {
        while (now_node->forward[now_level] != nullptr 
            && now_node->forward[now_level]->key < key) {
            now_node = now_node->forward[now_level];
        }
        pre_node[now_level] = now_node;
    }
    now_node = now_node->forward[0];

    // 찾았다면, 삭제 및 메모리 회수
    if (now_node != nullptr && now_node->key == key) {   
        for (int now_level = 0; now_level <= now_node->level; now_level++) { 
            if (pre_node[now_level]->forward[now_level] != now_node){
                break;
            }
            //비연결된 연결 재조립
            pre_node[now_level]->forward[now_level] = now_node->forward[now_level];
        }
        //메모리 회수
        delete[] now_node->forward;
        delete now_node;
    }
}

// CoarseSkipList 생성자
CoarseSkipList::CoarseSkipList(int max_level, float prob): DefaultSkipList(max_level, prob) {
  pthread_mutex_init(&mutex_lock, nullptr);
}

// CoarseSkipList 소멸자
CoarseSkipList::~CoarseSkipList() { 
    pthread_mutex_destroy(&mutex_lock); 
}

void CoarseSkipList::insert(int key, int value) {
    //동작하는 스킵리스트 자체에 락
    pthread_mutex_lock(&mutex_lock);
    
    //아래는 락 없는것과 동일

    // 현재 노드: now_node // 현재 노드의 이전 노드: pre_node
    Node *now_node = header_,  *pre_node[max_level_ + 1];

    //  포인터 초기화 작업업
    for (int loop = 0; loop <= max_level_; loop++){
        pre_node[loop] = nullptr; 
    }
    
    // 삽입 지점 순회
    // 레벨을 따라 검색하며, 삽입될 조건이 된다면(key가 작음) 결정
    for (int now_level = max_level_; now_level >= 0; now_level--) {   
        while (now_node->forward[now_level] != nullptr 
                && now_node->forward[now_level]->key < key) {
            now_node = now_node->forward[now_level];
        }
        pre_node[now_level] = now_node; 
    }

    now_node = now_node->forward[0]; 
    
    // 해당 키가 이미 존재하는 경우
    // 조건에 따라 갱신 후 종료
    if (now_node != nullptr && now_node->key == key) {
        now_node->value += value;
        now_node->upd_cnt++;
        pthread_mutex_unlock(&mutex_lock);
        return;
    }
    
    // 존재 하지 않는 경우 => 삽입입
    // 노드의 키, 값 등등 초기화
    int new_level = random_level();
    Node *insert_node = new Node();
    insert_node->forward = new Node *[new_level + 1];
    insert_node->level = new_level;
    insert_node->upd_cnt = 0;
    insert_node->key = key;
    insert_node->value = value;

    // 노드 삽입후, 이전과 연결
    for (int loop = 0; loop <= new_level; loop++) 
    {
        insert_node->forward[loop] = pre_node[loop]->forward[loop];
        pre_node[loop]->forward[loop] = insert_node;
    }

    // 락 해제
    pthread_mutex_unlock(&mutex_lock); 
}

int CoarseSkipList::lookup(int key) {
    // 리스트 전체 잠금금
    pthread_mutex_lock(&mutex_lock); 
    //순회용 노드 포인터
    Node* now_node = header_; 
    
    // 위치 추적: 작은 key
    for(int now_level = max_level_; now_level >= 0; now_level--) {
        while(now_node->forward[now_level] != nullptr 
            && now_node->forward[now_level]->key < key) {
            now_node = now_node->forward[now_level];
        }
    }
    now_node = now_node->forward[0];
    
    // 조건에 따라서 반환값 결정
    int result = 0;
    if (now_node != nullptr && now_node->key == key){
        result = now_node->value;
    }
    //락 해제
    pthread_mutex_unlock(&mutex_lock); 
    //값 반환
    return result; 
}

void CoarseSkipList::remove(int key) {
    //리스트 잠금
    pthread_mutex_lock(&mutex_lock); // 전체에 대한 lock
    //현재 노드 및 이전 노드
    Node *now_node = header_, *pre_node[max_level_ + 1];
    for (int loop = 0; loop <= max_level_; loop++){
        pre_node[loop] = nullptr;
    }
    
    // 삭제 노드 찾기
    for (int now_level = max_level_; now_level >= 0; now_level--) {
        while (now_node->forward[now_level] != nullptr 
            && now_node->forward[now_level]->key < key){
            now_node = now_node->forward[now_level];
        }
        pre_node[now_level] = now_node; 
    }
    now_node = now_node->forward[0]; 
    
    // 조건에 맞다면 삭제 및, 이전 노드 재연결 및 메모리 회수
    if (now_node != nullptr && now_node->key == key) {
        for (int loop = 0; loop <= now_node->level; loop++) {   
            if (pre_node[loop]->forward[loop] != now_node){
                break;
            }
            pre_node[loop]->forward[loop] = now_node->forward[loop];
        }
        //메모리 회수
        delete[] now_node->forward;
        delete now_node;
    }
    //리스트 잠금 해제
    pthread_mutex_unlock(&mutex_lock); 
}



/// 미구현(구현 '못'함)

// FineSkipList 생성자
FineSkipList::FineSkipList(int max_level, float prob) : DefaultSkipList(max_level, prob) {
    delete header_;
    header_ = new FineNode();
    header_->key = -1;
    header_->value = -1;
    header_->upd_cnt = 0;
    header_->level = max_level_;
    
    header_->forward = new Node*[max_level_ + 1];
    for (int level = 0; level <= max_level_; level++) {
        header_->forward[level] = nullptr;
    }
}


// FineSkipList 소멸자
FineSkipList::~FineSkipList() {
    FineNode* current = (FineNode*)header_;
    FineNode* temp;
    
    while(current) {
        temp = (FineNode*)current->forward[0];
        pthread_mutex_destroy(&((FineNode*)current)->lock);
        delete[] current->forward;
        delete current;
        current = temp;
    }
    
    header_ = nullptr;
}
void FineSkipList::insert(int key, int value) {
    // TODO
}

int FineSkipList::lookup(int key) {
    return 0;
}

void FineSkipList::remove(int key) {
    // TODO
}

