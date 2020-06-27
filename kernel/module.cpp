// The Tofita Kernel
// Copyright (C) 2020  Oleg Petrenko
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Implementation of hybrid kernel modules
// Modules are .dll files

struct Module {
	const char8_t *name;
	const RamDiskAsset *dll;
};

struct ModuleInterim {
	const char8_t *name;
	const RamDiskAsset *dll;
};

Module *loadModule(ModuleInterim *params) {
	var module = (Module *)PhysicalAllocator::allocateBytes(sizeof(Module));
	module->name = params->name; // TODO copy string
	module->dll = params->dll;

	// TODO relocation? or overwrite?
	auto ptr = (uint8_t *)params->dll->data;
	auto peHeader = (const exe::PeHeader *)((uint64_t)ptr + ptr[0x3C] + ptr[0x3C + 1] * 256);
	auto peOptionalHeader = (const exe::Pe32OptionalHeader *)((uint64_t)peHeader + sizeof(exe::PeHeader));
	void *base = (void *)peOptionalHeader->mImageBase;
	memset(base, 0, peOptionalHeader->mSizeOfImage); // Zeroing

	// Copy sections
	auto imageSectionHeader =
		(const exe::ImageSectionHeader *)((uint64_t)peOptionalHeader + peHeader->mSizeOfOptionalHeader);
	for (uint16_t i = 0; i < peHeader->mNumberOfSections; ++i) {
		serialPrintf(u8"Copy section [%d] named '%s' of size %d\n", i, &imageSectionHeader[i].mName,
					 imageSectionHeader[i].mSizeOfRawData);
		uint64_t where = (uint64_t)base + imageSectionHeader[i].mVirtualAddress;

		tmemcpy((void *)where,
				(void *)((uint64_t)params->dll->data + (uint64_t)imageSectionHeader[i].mPointerToRawData),
				imageSectionHeader[i].mSizeOfRawData);
	}

	return module;
}
