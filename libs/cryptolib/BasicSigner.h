
namespace av::crypto
{
	class Signer
	{
	public:
		using PublicKeyT  = CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::PublicKey;
		using PrivateKeyT = CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::PrivateKey;

	private:
		CryptoPP::ECDSA<CryptoPP::ECP,
			CryptoPP::SHA1>::PublicKey m_public_key;
	public:
		Signer(const std::string& private_key_blob);

		void generateKey();

		void doSmth();
	};
}
