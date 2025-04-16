#include "directory.hpp"
#include "cache.hpp"
#include <cassert>

namespace csim
{

    Directory::Directory(size_t num_procs, DirectoryCaches *caches, Stats *stats) : num_procs_(num_procs), caches_(caches), stats_(stats)
    {
        directory_ = std::unordered_map<size_t, DirEntry>();
    }

    void Directory::requestFromCache(DirMsg req)
    {
        DirMsgType type = req.type_;
        size_t src = req.src_proc_;
        size_t address = req.cpureq_.inst_.address;

        assert(type == DirMsgType::DATA || type == DirMsgType::WRITE || type == DirMsgType::UPGRADE);

        DirState currstate = GetState(address);

        if (type == DirMsgType::READ)
        {
            switch (currstate)
            {
            case DirState::UNCACHED:
            {
                // respond with full ownership
                DirMsg &resp = req;
                resp.type_ = DirMsgType::DATA;
                resp.src_proc_ = DIRECTORY;
                resp.dst_proc_ = src;
                caches_->replyFromDirectory(resp);

                // simulate traffic
                stats_->interconstats.traffic += (2 * 2);

                // record that a processor now owns this line.
                directory_[address] = DirEntry{.address = address,
                                               .owner = src,
                                               .sharers = std::unordered_set<size_t>(),
                                               .state = DirState::EXCLUSIVE};

                break;
            }
            case DirState::SHARED:
            {
                assert(!directory_[address].owner.has_value());
                assert(directory_[address].sharers.size() > 0);

                // respond with sharing
                DirMsg &resp = req;
                resp.type_ = DirMsgType::SHARED;
                resp.src_proc_ = DIRECTORY;
                resp.dst_proc_ = src;
                caches_->replyFromDirectory(resp);

                // simulate traffic
                stats_->interconstats.traffic += (5 * 2);

                // Add to list sharers
                directory_[address].sharers.insert(src);
                break;
            }

            case DirState::EXCLUSIVE:
            {
                assert(directory_[address].owner.has_value());
                assert(directory_[address].sharers.empty());

                // downgrade current owner
                size_t owner = directory_[address].owner.value();
                DirMsg downgradereq = req;
                downgradereq.type_ = DirMsgType::SHARED;
                downgradereq.src_proc_ = DIRECTORY;
                downgradereq.dst_proc_ = owner;
                caches_->requestFromDirectory(downgradereq);

                // remove owner and add it to sharers
                directory_[address].owner.reset();
                directory_[address].sharers.insert(owner);

                // reply with shared to current requestor
                DirMsg sharedresp = req;
                sharedresp.type_ = DirMsgType::SHARED;
                sharedresp.src_proc_ = DIRECTORY;
                sharedresp.dst_proc_ = src;
                caches_->replyFromDirectory(sharedresp);

                // add requestor to sharers list
                directory_[address].sharers.insert(src);

                // change state to shared
                directory_[address].state = DirState::SHARED;

                // simulate traffic
                stats_->interconstats.traffic += (5 * 2);
                break;
            }
            }
        }
        else if (type == DirMsgType::WRITE)
        {
            switch (currstate)
            {
            case DirState::UNCACHED:
            {
                // respond with full ownership
                DirMsg &resp = req;
                resp.type_ = DirMsgType::DATA;
                resp.src_proc_ = DIRECTORY;
                resp.dst_proc_ = src;
                caches_->replyFromDirectory(resp);

                // simulate traffic
                stats_->interconstats.traffic += (2 * 2);

                // record that a processor now owns this line.
                directory_[address] = DirEntry{.address = address,
                                               .owner = src,
                                               .sharers = std::unordered_set<size_t>(),
                                               .state = DirState::EXCLUSIVE};

                break;
            }
            case DirState::SHARED:
            {
                assert(!directory_[address].owner.has_value());
                assert(directory_[address].sharers.size() > 0);

                size_t sharerscount = directory_[address].sharers.size();

                // invalidate all sharers
                for (size_t sharer : directory_[address].sharers)
                {
                    DirMsg invlreq = req;
                    invlreq.type_ = DirMsgType::INVALIDATE;
                    invlreq.src_proc_ = DIRECTORY;
                    invlreq.dst_proc_ = sharer;
                    caches_->requestFromDirectory(invlreq);
                }

                // remove the shared ownership
                directory_[address].sharers.clear();

                // respond to requestor with full ownership
                DirMsg resp = req;
                resp.type_ = DirMsgType::DATA;
                resp.src_proc_ = DIRECTORY;
                resp.dst_proc_ = src;
                caches_->replyFromDirectory(resp);

                // record that the requestor now owns this line
                directory_[address].owner = src;
                directory_[address].state = DirState::EXCLUSIVE;

                // simulate traffic
                stats_->interconstats.traffic += ((2 + (sharerscount * 2)) * 2);

                break;
            }
            case DirState::EXCLUSIVE:
            {
                assert(directory_[address].owner.has_value());
                assert(directory_[address].sharers.empty() == 0);

                // invalidate current owner
                size_t currowner = directory_[address].owner.value();
                DirMsg invlreq = req;
                invlreq.type_ = DirMsgType::INVALIDATE;
                invlreq.src_proc_ = DIRECTORY;
                invlreq.dst_proc_ = currowner;
                caches_->requestFromDirectory(invlreq);

                // remove the current ownership
                directory_[address].owner.reset();

                // respond to requestor with full ownership
                DirMsg resp = req;
                resp.type_ = DirMsgType::DATA;
                resp.src_proc_ = DIRECTORY;
                resp.dst_proc_ = src;
                caches_->replyFromDirectory(resp);

                // record that the requestor now owns this line
                directory_[address].owner = src;
                directory_[address].state = DirState::EXCLUSIVE;

                // simulate traffic
                stats_->interconstats.traffic += ((2 + (1 * 2)) * 2);
                break;
            }
            }
        }
        else if (type == DirMsgType::UPGRADE)
        {
            switch (currstate)
            {
            case DirState::UNCACHED:
            {
                std::cerr << "Something is broken" << std::endl;
                assert(false);
                break;
            }
            case DirState::SHARED:
            {
                assert(!directory_[address].owner.has_value());
                assert(directory_[address].sharers.size() > 0);

                size_t sharerscount = directory_[address].sharers.size() - 1;

                // invalidate all sharers except the requestor
                for (size_t sharer : directory_[address].sharers)
                {
                    if (sharer != src)
                    {
                        DirMsg invlreq = req;
                        invlreq.type_ = DirMsgType::INVALIDATE;
                        invlreq.src_proc_ = DIRECTORY;
                        invlreq.dst_proc_ = sharer;
                        caches_->requestFromDirectory(invlreq);
                    }
                }

                // remove the shared ownership
                directory_[address].sharers.clear();

                // respond to requestor with full ownership
                DirMsg resp = req;
                resp.type_ = DirMsgType::DATA;
                resp.src_proc_ = DIRECTORY;
                resp.dst_proc_ = src;
                caches_->replyFromDirectory(resp);

                // record that the requestor now owns this line
                directory_[address].owner = src;
                directory_[address].state = DirState::EXCLUSIVE;

                // simulate traffic
                stats_->interconstats.traffic += ((2 + (sharerscount * 2)) * 2);
                break;
            }
            case DirState::EXCLUSIVE:
            {
                std::cerr << "Something is broken" << std::endl;
                assert(false);
                break;
            }
            }
        }
    }
    DirState Directory::GetState(size_t address)
    {
        if (directory_.find(address) == directory_.end())
        {
            return DirState::UNCACHED;
        }

        return directory_[address].state;
    }

    void Directory::SetState(size_t address, size_t proc, DirState newstate)
    {
    }
}
