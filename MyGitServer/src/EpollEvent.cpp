#include "EpollEvent.h"
#include "Tool.h"
#include<unistd.h>
#include<cstring>

const int MAX_EVENTS = 1024;

Epoll::Epoll(){
    epoll_fd = epoll_create1(0);
    err_if(epoll_fd == -1, "epoll_create1 failed");
    epoll_events=new epoll_event[MAX_EVENTS];
    bzero(epoll_events, sizeof(*epoll_events)*MAX_EVENTS);
}

Epoll::~Epoll(){
    if(epoll_fd!= -1){
        close(epoll_fd);
        epoll_fd=-1;
    }
    delete[] epoll_events;
}

void Epoll::quit_loop(){
    quit=true;
}

void Epoll::update_channel(Channel* channel){
    int channel_fd = channel->get_channel_fd();

    epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->get_events();

    if(!channel->is_in_epoll()){
        err_if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, channel_fd, &ev) == -1, "epoll_ctl add failed");
        channel->set_in_epoll(true);
    }
    else{
        err_if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, channel_fd, &ev) == -1, "epoll_ctl mod failed");
    }
}

void Epoll::remove_channel(Channel* channel){
    int channel_fd = channel->get_channel_fd();
    err_if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, channel_fd, NULL) == -1, "epoll_ctl del failed");
}

void Epoll::loop(){
    while(!quit){
        vector<Channel*>channels;
        channels=this->poll();
        for(auto it=channels.begin(); it!=channels.end(); it++){
            (*it)->handle_event();
        }
    }
}

vector<Channel*> Epoll::poll(int timeout){
    vector<Channel*> ready_channels;
    int nfds=epoll_wait(epoll_fd, epoll_events, MAX_EVENTS, timeout);
    err_if(nfds == -1, "epoll_wait failed");
    for(int i=0; i<nfds; i++){
        Channel*channel=(Channel*)epoll_events[i].data.ptr;
        channel->set_events(epoll_events[i].events);
        ready_channels.push_back(channel);
    }
    return ready_channels;
}





Channel::Channel(Epoll* epoll_loop, int channel_fd)
:epoll_loop(epoll_loop),channel_fd(channel_fd),events(0),in_epoll(false){
    
}

Channel::~Channel(){
    if(channel_fd!=-1){
        close(channel_fd);
        channel_fd=-1;
    }
}

void Channel::handle_event(){
    if(events&(EPOLLIN|EPOLLPRI)){
        read_callback();
    }
    if(events&EPOLLOUT)
        write_callback();
}

void Channel::enable_read(){
    events|=EPOLLIN|EPOLLPRI;
    epoll_loop->update_channel(this);
}

void Channel::set_ET(){
    events|=EPOLLET;
    epoll_loop->update_channel(this);
}

void Channel::disable_read(){
    events&=~(EPOLLIN|EPOLLPRI);
    epoll_loop->update_channel(this);
}

int Channel::get_channel_fd(){
    return channel_fd;
}

void Channel::set_read_callback(function<void()> cb) {
    read_callback = cb;
}

void Channel::set_write_callback(function<void()> cb) {
    write_callback = cb;
}

uint32_t Channel::get_events() {
    return events;
}


bool Channel::is_in_epoll() {
    return in_epoll;
}

void Channel::set_in_epoll(bool in_epoll) {
    this->in_epoll = in_epoll;
}

void Channel::set_events(uint32_t events) {
    this->events = events;
}

