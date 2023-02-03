#pragma once

struct Photon {
    float3 flux;
    float3 position;
    float3 direction;
    static const int DIM = 3;

    float operator[](int i) const { return position.cell[i]; }

    Photon(const float3& flux, const float3& position, const float3& direction)
        : flux(flux), position(position), direction(direction) {}
};

class PhotonMap {
public:
    PhotonMap();

    const int       GetPhotonsCount() const;
    const Photon&   GetPhoton(int i) const;
    void            AddPhoton(const Photon& photon);

    void            Build();

    std::vector<int> QueryKNearestPhotons(const Photon& p, int k) const;

private:
    std::vector<Photon> photons;
    KDTree<Photon> kdtree;
};
