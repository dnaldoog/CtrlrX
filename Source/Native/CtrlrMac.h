#ifndef __CTRLR_MAC__
#define __CTRLR_MAC__

#include "CtrlrNative.h"

typedef struct vmtotal vmtotal_t;

class CtrlrMac : public CtrlrNative
{
	public:
		CtrlrMac(CtrlrManager &_owner);
		~CtrlrMac();
		const Result exportWithDefaultPanel(CtrlrPanel*  panelToWrite, const bool isRestricted=false, const bool signPanel=false);
		const Result getDefaultPanel(MemoryBlock &dataToWrite);
		const Result getDefaultResources(MemoryBlock& dataToWrite);
		const Result setBundleInfo (CtrlrPanel *sourceInfo, const File &bundle);
		const Result setBundleInfoCarbon (CtrlrPanel *sourceInfo, const File &bundle);

    private:
        CtrlrManager &owner;
        std::unique_ptr<FileChooser> fc; // Added v5.6.31
        std::unique_ptr<FileChooser> fileChooser; // Added v5.6.31
        std::unique_ptr<FileChooser> chosenFolder; // Added v5.6.31
};
#endif
