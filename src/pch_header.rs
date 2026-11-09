//! Proof-Carrying Hardware (.pch) Container Specification & Lightweight Parser (v2)
//!
//! Author: Srijan Mandal <srijanmandal@leibniz.systems>
//! Repository: Leibniz Systems / Probare

use sha2::{Digest, Sha256};
use std::fmt;

pub const PCH_MAGIC: &[u8; 4] = b"PCH2";
pub const PCH_HEADER_LEN: usize = 256;
pub const PCH_VERSION_2: u16 = 2;

#[repr(u16)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum IsaTarget {
    Avx512F = 1,
    Avx512VlDq = 2,
    ArmNeon = 3,
    FreestandingX86_64 = 4,
    Unknown = 0xFFFF,
}

impl From<u16> for IsaTarget {
    fn from(val: u16) -> Self {
        match val {
            1 => IsaTarget::Avx512F,
            2 => IsaTarget::Avx512VlDq,
            3 => IsaTarget::ArmNeon,
            4 => IsaTarget::FreestandingX86_64,
            _ => IsaTarget::Unknown,
        }
    }
}

/// Exact 256-byte binary header definition as described in Table 4 of the Probare paper.
#[derive(Clone, PartialEq, Eq)]
pub struct PchHeaderV2 {
    pub magic: [u8; 4],
    pub version: u16,
    pub isa_target: IsaTarget,
    pub solver_mask: u16,
    pub ir_sha256: [u8; 32],
    pub bin_sha256: [u8; 32],
    pub raw_header: [u8; PCH_HEADER_LEN],
    pub signature: [u8; 64],
}

impl fmt::Debug for PchHeaderV2 {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("PchHeaderV2")
            .field("magic", &String::from_utf8_lossy(&self.magic))
            .field("version", &self.version)
            .field("isa_target", &self.isa_target)
            .field("solver_mask", &format_args!("0x{:04X}", self.solver_mask))
            .field("ir_sha256", &hex::encode(self.ir_sha256))
            .field("bin_sha256", &hex::encode(self.bin_sha256))
            .finish()
    }
}

#[derive(Debug, PartialEq, Eq)]
pub enum PchValidationError {
    InvalidMagic,
    UnsupportedVersion(u16),
    LengthMismatch(usize),
    BinaryDigestMismatch,
    SignatureInvalid,
}

impl fmt::Display for PchValidationError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::InvalidMagic => write!(f, "Invalid PCH header magic bytes"),
            Self::UnsupportedVersion(v) => write!(f, "Unsupported PCH format version: {}", v),
            Self::LengthMismatch(l) => write!(f, "Header slice length mismatch: expected 256, got {}", l),
            Self::BinaryDigestMismatch => write!(f, "Cryptographic hash of machine code does not match bin_sha256 in header"),
            Self::SignatureInvalid => write!(f, "Ed25519 signature over header failed verification"),
        }
    }
}

impl std::error::Error for PchValidationError {}

impl PchHeaderV2 {
    /// Parses a 256-byte header slice.
    pub fn parse(bytes: &[u8]) -> Result<Self, PchValidationError> {
        if bytes.len() != PCH_HEADER_LEN {
            return Err(PchValidationError::LengthMismatch(bytes.len()));
        }

        if &bytes[0..4] != PCH_MAGIC {
            return Err(PchValidationError::InvalidMagic);
        }

        let version = u16::from_le_bytes([bytes[4], bytes[5]]);
        if version != PCH_VERSION_2 {
            return Err(PchValidationError::UnsupportedVersion(version));
        }

        let isa_target = IsaTarget::from(u16::from_le_bytes([bytes[6], bytes[7]]));
        let solver_mask = u16::from_le_bytes([bytes[12], bytes[13]]);

        let mut ir_sha256 = [0u8; 32];
        ir_sha256.copy_from_slice(&bytes[16..48]);

        let mut bin_sha256 = [0u8; 32];
        bin_sha256.copy_from_slice(&bytes[48..80]);

        let mut signature = [0u8; 64];
        signature.copy_from_slice(&bytes[192..256]);

        let mut raw_header = [0u8; PCH_HEADER_LEN];
        raw_header.copy_from_slice(bytes);

        Ok(Self {
            magic: *PCH_MAGIC,
            version,
            isa_target,
            solver_mask,
            ir_sha256,
            bin_sha256,
            raw_header,
            signature,
        })
    }

    /// Validates the binary payload against the recorded bin_sha256 digest.
    pub fn verify_payload_digest(&self, machine_code: &[u8]) -> Result<(), PchValidationError> {
        let mut hasher = Sha256::new();
        hasher.update(machine_code);
        let digest: [u8; 32] = hasher.finalize().into();

        if digest == self.bin_sha256 {
            Ok(())
        } else {
            Err(PchValidationError::BinaryDigestMismatch)
        }
    }
}
