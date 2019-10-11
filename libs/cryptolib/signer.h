
namespace av::crypto
{
	class BasicSigner
	{
	public:

		using BaseT       = CryptoPP::ECDSA<CryptoPP::EC2N, CryptoPP::SHA512>;
	    using PublicKeyT  = BaseT::PrivateKey;
		using PrivateKeyT = BaseT::PublicKey;
		using SignerT     = BaseT::Signer;
        using VerifierT   = BaseT::Verifier;
        

	private:

	    PublicKeyT  m_public_key;
		PrivateKeyT m_private_key;

	public:
		explicit BasicSigner();


		void generateKeyPair();
		void exportPublicKey();
		void exportPrivateKey();
		void importPrivateKey();

		virtual ~BasicSigner();
	};
}
