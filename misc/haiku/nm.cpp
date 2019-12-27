#include <Directory.h>
#include <Looper.h>
#include <NodeMonitor.h>
#include <MessageFilter.h>

#include "haiku_interop.h"

filter_result dir_mon_flt(BMessage *message, BHandler **hnd, BMessageFilter *fltr) {
	(void) hnd;
	(void) fltr;

	if (message->what == B_NODE_MONITOR) {
		int32 val;
		message->FindInt32("opcode", &val);

		switch (val) {
			case B_ENTRY_CREATED:
			case B_ENTRY_MOVED:
			case B_ENTRY_REMOVED:
				return B_DISPATCH_MESSAGE;
		}
	}

	return B_SKIP_MESSAGE;
}

class DirectoryListener : public BLooper {
public:
	bool recv_reset() {
		Lock();
		bool val = _ev_on;
		_ev_on = false;
		Unlock();

		return val;
	}
private:
	void MessageReceived(BMessage * message) override {
		Lock();
		_ev_on = true;
		Unlock();
		BLooper::MessageReceived(message);
	}

	bool _ev_on = false;
};

struct haiku_nm_t {
	haiku_nm_t() {
		dl = new DirectoryListener();
		flt = new BMessageFilter(B_PROGRAMMED_DELIVERY, B_LOCAL_SOURCE, dir_mon_flt);
		dl->AddCommonFilter(flt);
		dl->Run();
	}

	DirectoryListener *dl;
	BMessageFilter *flt;
	node_ref nr;
};

haiku_nm_h haiku_init_nm() {
	return new haiku_nm_t();
}

void haiku_close_nm(haiku_nm_h hnd) {
	delete hnd->flt;
	// This is the way of deleting a BLooper
	hnd->dl->PostMessage(B_QUIT_REQUESTED);
	delete hnd;
}
int haiku_watch_dir(haiku_nm_h hnd, const char *path) {
	BDirectory dir(path);
	dir.GetNodeRef(&(hnd->nr));

	return watch_node(&(hnd->nr), B_WATCH_DIRECTORY, nullptr, hnd->dl);
}
int haiku_stop_watch(haiku_nm_h hnd) {
	return watch_node(&(hnd->nr), B_STOP_WATCHING, nullptr, hnd->dl);
}

int haiku_is_update_needed(haiku_nm_h hnd) {
	return hnd->dl->recv_reset();
}
