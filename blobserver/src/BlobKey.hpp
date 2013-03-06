
#ifndef __BLOBSERVER_BLOB_KEY_INDEX_H__
#define __BLOBSERVER_BLOB_KEY_INDEX_H__

#include <string>
#include <boost/optional.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace blobserver {

	class BlobKey {

		public:
			explicit BlobKey(std::string hash_type, std::string hash_value);\
			~BlobKey();

			// NKG: Consider making this not a friend method.
			friend 	bool operator<(const BlobKey& mk1, const BlobKey& mk2) {
				if (mk1.hash_type_ != mk2.hash_type_ ) {
					return mk1.hash_type_ < mk2.hash_type_;
				}

				return mk1.hash_value_ < mk2.hash_value_;
			}

			std::string hash_type() const {
				return hash_type_;
			}

			std::string hash_value() const {
				return hash_value_;
			}

			std::string blobref() {
				return hash_type_ + "-" + hash_value_;
			}

		private:
			std::string hash_type_;
			std::string hash_value_;

			friend class boost::serialization::access;

			// When the class Archive corresponds to an output archive, the
			// & operator is defined similar to <<.  Likewise, when the class Archive
			// is a type of input archive the & operator is defined similar to >>.
			template<class Archive>
			void serialize(Archive & ar, const unsigned int version) {
				ar & hash_type_;
				ar & hash_value_;
			}

	};

	boost::optional<BlobKey> create_blob_key(std::string hash);

	// NKG: Consider moving this to the operator== method within the BlobKey class.
	struct cmpr_BlobKey {
		bool operator()(const BlobKey& mk1, const BlobKey& mk2) {
			return mk1.hash_type() == mk2.hash_type() && mk1.hash_value() == mk2.hash_value();
		}
	};

}

#endif
