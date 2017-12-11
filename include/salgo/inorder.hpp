#pragma once



namespace salgo {



namespace internal {

	//
	// used for INORDER binary tree traversal
	//
	// operator-- is disabled to prevent users from computing begin() too often
	//
	template<class TREE>
	class Inorder {
	private:
		static constexpr bool is_const = !std::is_same_v< std::remove_const_t<TREE>, TREE>;
		static constexpr Const_Flag C = is_const ? CONST : MUTAB;

	public:
		Inorder(TREE& tree) : _tree(tree) {
			static_assert(TREE::Parent_Links, "currently inorder traversal requires parent links");
		}

		auto begin() const {
			return Iterator(_tree.begin(), false);
		}

		auto end() const {
			return Iterator(_tree.end(), true);
		}


	private:

		/*
		template<Const_Flag C>
		class Accessor {
		public:
			Accessor()
		};
		*/


		//
		#define SELF (**this)

		class Iterator : public TREE::template Iterator_Base<C, Iterator> {
		private:
			using BASE = typename TREE::template Iterator_Base<C, Iterator>;
			using BASE::owner;
			using BASE::idx;

		public:
			Iterator(const typename TREE::template Iterator<C>& base_iter, bool end)
					: BASE(base_iter) {

				// end
				if(end) {
					idx = decltype(idx)();
				}
				else if((*base_iter).exists) {
					while(SELF.parent) {
						idx = SELF.parent().key;
					}

					while(SELF.left) {
						idx = SELF.left().key;
					}
				}
			}

		public:
			using BASE::operator==;
			using BASE::operator!=;


			auto& operator++() {
				increment();
				return *this; }

			auto operator++(int) {
				auto old = *this;
				increment();
				return old; }

		private:
			template<bool B = false>
			auto& operator--() { static_assert(B, "operator--() is disabled"); }

			template<bool B = false>
			auto operator--(int) { static_assert(B, "operator--(int) is disabled"); }


		private:
			void increment() {

				if( SELF.right ) {
					idx = SELF.right().key;

					while( SELF.left )  idx = SELF.left().key;
				}
				else {
					for(;;) {
						auto prev_idx = idx;
						idx = SELF.parent().key;

						if(idx != decltype(BASE::idx)() && SELF.right && SELF.right().key == prev_idx) continue;
						else break;
					}
				}
			}


			// currently never used:
			/*
			void decrement() {

				if(idx > 0) {
					if( SELF.left ) {
						idx = SELF.left().key;

						while( SELF.right )  idx = SELF.right().key;
					}
					else {
						do {
							auto prev_idx = idx;
							idx = SELF.parent().key;
						} while(idx > 0 && SELF.left && SELF.left().key == prev_idx);
					}
				}
				else { // idx == 0
					while( SELF.right )  idx = SELF.right().key;
				}
			}
			*/
		};

		#undef SELF
		//



	private:
		TREE& _tree;
	};


} // namespace internal





template<class TREE>
class Inorder : public internal::Inorder<TREE> {
public:
	// TODO: implement const version
	Inorder(TREE& tree) : internal::Inorder<TREE>(tree) {}
};






} // namespace salgo

