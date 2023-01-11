#include "precomp.h"

PhotonMap::PhotonMap()
{

}

const int PhotonMap::GetPhotonsCount() const
{
	return photons.size();
}

const Photon& PhotonMap::GetPhoton(int i) const
{
	return photons[i];
}

void PhotonMap::AddPhoton(const Photon& photon)
{
	photons.push_back(photon);
}

void PhotonMap::Build()
{
	kdtree.build(photons);
}

std::vector<int> PhotonMap::QueryKNearestPhotons(const Photon& p, int k) const
{
	return kdtree.knnSearch(p, k);
}
