#include <bits/stdc++.h>

using namespace std;
void b(vector<int>& nums,int i);
void c(vector<int>& nums,int i);
void d(vector<int>& nums,int i);
void e(vector<int>& nums,int i);
void a(vector<int>& nums,int i){
    cout<<"a-"<<nums[0]<<" i="<<i<<endl;
    nums[1]++;
    if(i==nums.size()-1){
        if(nums.size()-4<=0)return;
        e(nums,nums.size()-4);
        return;
    }
}
void b(vector<int>& nums,int i){
    cout<<"b-"<<nums[1]<<" i="<<i<<endl;
    nums[2]++;
    if(i==nums.size()-1){
        if(nums.size()-3<=0)return;
        e(nums,nums.size()-3);
        return;
    }
    a(nums,i+1);
}
void c(vector<int>& nums,int i){
    cout<<"c-"<<nums[2]<<" i="<<i<<endl;
    nums[3]++;
    if(i==nums.size()-1){
        if(nums.size()-2<=0)return;
        e(nums,nums.size()-2);
        return;
    }
    b(nums,i+1);
}
void d(vector<int>& nums,int i){
    cout<<"d-"<<nums[3]<<" i="<<i<<endl;
    nums[4]++;
    if(i==nums.size()-1){
        if(nums.size()-1<=0)return;
        e(nums,nums.size()-1);
        return;
    }
    c(nums,i+1);
}
void e(vector<int>& nums,int i){
    cout<<"e-"<<nums[4]<<" i="<<i<<endl;
    if(i==nums.size()-1)return;
    d(nums,i+1);
}

int main(){
    vector<int> nums={1,2,7,4,5};
    a(nums,0);
    b(nums,0);
    c(nums,0);
    d(nums,0);
    e(nums,0);
    return 0;
}