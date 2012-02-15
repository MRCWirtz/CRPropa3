#include "mpc/ModuleChain.h"

namespace mpc {

const ModuleChain::list_t &ModuleChain::getStartModules() const {
	return startModules;
}
const ModuleChain::list_t &ModuleChain::getMainModules() const {
	return mainModules;
}
const ModuleChain::list_t &ModuleChain::getEndModules() const {
	return endModules;
}

void ModuleChain::add(Module *module, size_t priority) {
	if (module == 0)
		return;

	list_entry_t entry;
	entry.first = priority;
	entry.second = module;

	if (priority == Priority::Start) {
		startModules.push_back(entry);
	} else if (priority == Priority::End) {
		endModules.push_back(entry);
	} else {
		mainModules.push_back(entry);
		mainModules.sort();
	}

	check();
}

void ModuleChain::clear() {
	startModules.clear();
	mainModules.clear();
	endModules.clear();
}

void ModuleChain::check() {
	size_t integratorCount = 0;

	list_t::iterator iEntry = mainModules.begin();
	while (iEntry != mainModules.end()) {
		list_entry_t &entry = *iEntry;
		iEntry++;

		if (entry.first == Priority::Propagation) {
			integratorCount++;
		}
	}

	if (integratorCount > 1) {
		std::cerr << "Warning: more than one propagation feature present."
				<< std::endl;
	}
}

void ModuleChain::process(list_t &list, Candidate *candidate,
		std::vector<Candidate *> &secondaries) {
	list_t::iterator iEntry = list.begin();
	while (iEntry != list.end()) {
		list_entry_t &entry = *iEntry;
		iEntry++;
		entry.second->process(candidate, secondaries);
	}
}

void ModuleChain::process(Candidate *candidate,
		std::vector<Candidate *> &secondaries) {
	std::cout << "[ModuleChain::process] propagate" << std::endl;
	if (mainModules.size() == 0)
		return;

	process(startModules, candidate, secondaries);

	while (candidate->getStatus() == Candidate::Active) {
		if (mainModules.size() == 0)
			break;
		process(mainModules, candidate, secondaries);
	}

	process(endModules, candidate, secondaries);
}

void ModuleChain::process(std::vector<Candidate *> &candidates,
		bool recursive) {
	std::vector<Candidate *> secondaries;
	bool haveActive = true;
	while (haveActive) {
		haveActive = false;
		std::cout << "[ModuleChain::process] number of candidates"
				<< candidates.size() << std::endl;
		for (size_t i = 0; i < candidates.size(); i++) {
			Candidate *candidate = candidates[i];
			if (candidate->getStatus() != Candidate::Active)
				continue;
			process(candidate, secondaries);
			haveActive = true;

			// propagate secondaries
			if (recursive)
				this->process(candidate->secondaries, recursive);
		}
	}
}

} // namespace mpc

std::ostream &operator<<(std::ostream &out, const mpc::ModuleChain &chain) {
	mpc::ModuleChain::list_t::const_iterator iEntry =
			chain.getMainModules().begin();
	while (iEntry != chain.getMainModules().end()) {
		const mpc::ModuleChain::list_entry_t &entry = *iEntry;
		iEntry++;

		out << entry.first << " -> " << entry.second->getDescription() << "\n";
	}
	return out;
}
