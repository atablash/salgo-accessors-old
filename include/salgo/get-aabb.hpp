#pragma once


#include <Eigen/Dense>


namespace salgo {







template<class SCALAR, int DIM>
inline const auto& get_aabb(const Eigen::AlignedBox<SCALAR,DIM>& aligned_box) {
	return aligned_box;
}


template<class SCALAR, int DIM>
inline auto get_aabb(const Eigen::Matrix<SCALAR, DIM, 1>& point) {
	return Eigen::AlignedBox<SCALAR,DIM>(point, point);
}



// TODO: same for sphere











} // namespace salgo



